#include "library_group_organizer.hpp"

#include <QSqlError>
#include <QSqlQuery>

#include <algorithm>

#include "dictionary_summary.hpp"
#include "schema_migration.hpp"
#include "section_type.hpp"
#include "text_embedding.hpp"
#include "text_similarity.hpp"

namespace lexis {

namespace {

// Two clusters are merged while their centroids are at least this similar.
constexpr double kMergeThreshold = 0.45;
// Label candidates come from the most frequent vocabulary words, skipping the
// very top ranks that are dominated by function words ("the", "of", ...).
constexpr int kLabelCandidatePool = 8000;
constexpr int kLabelMinRank = 200;

struct LooseWord {
  int itemId = -1;
  QString title;
  EmbeddingVector vector;
};

struct Cluster {
  QVector<int> members;  // indices into the LooseWord list
  EmbeddingVector centroid;
};

QVector<LooseWord> loadLooseWords(const QSqlDatabase& db, const QString& languageCode,
                                  int scopeRootId) {
  QVector<LooseWord> words;

  QSqlQuery query(db);
  if (scopeRootId == kRootParentId) {
    query.prepare(
      "SELECT id, title, meaning, dictionary_summary, cached_translation FROM items "
      "WHERE language_code = :language_code AND type = :type AND parent_id IS NULL");
  } else {
    query.prepare(
      "SELECT id, title, meaning, dictionary_summary, cached_translation FROM items "
      "WHERE language_code = :language_code AND type = :type AND parent_id = :parent_id");
    query.bindValue(":parent_id", scopeRootId);
  }
  query.bindValue(":language_code", languageCode);
  query.bindValue(":type", std::to_underlying(LibrarySectionType::kWord));

  if (!query.exec()) {
    qWarning() << "Failed to load loose words for group proposals:" << query.lastError();
    return words;
  }

  while (query.next()) {
    LooseWord word;
    word.itemId = query.value("id").toInt();
    word.title = query.value("title").toString();

    word.vector = TextEmbedding::embedText(languageCode, word.title);
    if (word.vector.isEmpty()) {
      const auto context = combineSemanticContext(query.value("meaning").toString(),
                                                  query.value("dictionary_summary").toString(),
                                                  query.value("cached_translation").toString());
      word.vector = TextEmbedding::embedText(languageCode, context);
    }
    if (!word.vector.isEmpty()) {
      words.append(std::move(word));
    }
  }

  return words;
}

void recomputeCentroid(Cluster& cluster, const QVector<LooseWord>& words) {
  QVector<EmbeddingVector> vectors;
  vectors.reserve(cluster.members.size());
  for (const auto memberIndex : cluster.members) {
    vectors.append(words[memberIndex].vector);
  }
  cluster.centroid = TextEmbedding::meanVector(vectors);
  TextEmbedding::normalize(cluster.centroid);
}

// Greedy average-linkage agglomerative clustering. Word counts per folder are
// small, so the cubic worst case is not a concern.
QVector<Cluster> clusterWords(const QVector<LooseWord>& words) {
  QVector<Cluster> clusters;
  clusters.reserve(words.size());
  for (int i = 0; i < words.size(); ++i) {
    clusters.append({{i}, words[i].vector});
  }

  while (clusters.size() > 1) {
    double bestSimilarity = kMergeThreshold;
    int bestLeft = -1;
    int bestRight = -1;

    for (int left = 0; left < clusters.size(); ++left) {
      for (int right = left + 1; right < clusters.size(); ++right) {
        const auto similarity =
          TextEmbedding::similarity(clusters[left].centroid, clusters[right].centroid);
        if (similarity >= bestSimilarity) {
          bestSimilarity = similarity;
          bestLeft = left;
          bestRight = right;
        }
      }
    }

    if (bestLeft < 0) {
      break;
    }

    clusters[bestLeft].members.append(clusters[bestRight].members);
    clusters.removeAt(bestRight);
    recomputeCentroid(clusters[bestLeft], words);
  }

  return clusters;
}

double clusterCohesion(const Cluster& cluster, const QVector<LooseWord>& words) {
  if (cluster.members.isEmpty()) {
    return 0.0;
  }
  double total = 0.0;
  for (const auto memberIndex : cluster.members) {
    total += TextEmbedding::similarity(words[memberIndex].vector, cluster.centroid);
  }
  return total / cluster.members.size();
}

bool isUsableLabel(const QString& candidate, const Cluster& cluster,
                   const QVector<LooseWord>& words) {
  for (const auto memberIndex : cluster.members) {
    if (substringSimilarity(candidate, words[memberIndex].title) > 0.0) {
      return false;
    }
  }
  return true;
}

// Cohesion is a raw cosine between members and their centroid; strip the
// noise floor before presenting it as a 0-100 confidence.
int confidenceFromCohesion(double cohesion) {
  const auto normalized = (cohesion - 0.30) / 0.70;
  const auto clamped = normalized < 0.0 ? 0.0 : (normalized > 1.0 ? 1.0 : normalized);
  return qRound(100.0 * clamped);
}

QString capitalized(const QString& word) {
  if (word.isEmpty()) {
    return word;
  }
  return word.front().toUpper() + word.mid(1);
}

QString proposeLabel(const QString& languageCode, const Cluster& cluster,
                     const QVector<LooseWord>& words) {
  const auto candidates = EmbeddingLookup::topWords(languageCode, kLabelCandidatePool);

  QString bestLabel;
  double bestSimilarity = 0.0;
  for (const auto& candidate : candidates) {
    if (candidate.rank <= kLabelMinRank) {
      continue;
    }
    const auto similarity = TextEmbedding::similarity(candidate.vector, cluster.centroid);
    if (similarity <= bestSimilarity) {
      continue;
    }
    if (!isUsableLabel(candidate.word, cluster, words)) {
      continue;
    }
    bestLabel = candidate.word;
    bestSimilarity = similarity;
  }

  if (!bestLabel.isEmpty()) {
    return capitalized(bestLabel);
  }

  // Fall back to the most central member word.
  int bestMember = cluster.members.front();
  double bestMemberSimilarity = -1.0;
  for (const auto memberIndex : cluster.members) {
    const auto similarity =
      TextEmbedding::similarity(words[memberIndex].vector, cluster.centroid);
    if (similarity > bestMemberSimilarity) {
      bestMemberSimilarity = similarity;
      bestMember = memberIndex;
    }
  }
  return capitalized(words[bestMember].title);
}

}  // namespace

QVariantList LibraryGroupOrganizer::proposeGroups(const QSqlDatabase& db,
                                                  const QString& languageCode, int scopeRootId,
                                                  int minClusterSize, int maxProposals) {
  QVariantList results;
  if (languageCode.isEmpty() || minClusterSize < 2 || maxProposals <= 0 ||
      !EmbeddingLookup::isOpen()) {
    return results;
  }

  const auto words = loadLooseWords(db, languageCode, scopeRootId);
  if (words.size() < minClusterSize) {
    return results;
  }

  auto clusters = clusterWords(words);
  clusters.erase(std::remove_if(clusters.begin(), clusters.end(),
                                [minClusterSize](const Cluster& cluster) {
                                  return cluster.members.size() < minClusterSize;
                                }),
                 clusters.end());
  if (clusters.isEmpty()) {
    return results;
  }

  QVector<QPair<double, int>> ranking;  // (cohesion, cluster index)
  ranking.reserve(clusters.size());
  for (int i = 0; i < clusters.size(); ++i) {
    ranking.append({clusterCohesion(clusters[i], words), i});
  }
  std::sort(ranking.begin(), ranking.end(), [&clusters](const auto& left, const auto& right) {
    const auto leftSize = clusters[left.second].members.size();
    const auto rightSize = clusters[right.second].members.size();
    if (leftSize != rightSize) {
      return leftSize > rightSize;
    }
    return left.first > right.first;
  });

  const int resultCount = std::min<int>(maxProposals, ranking.size());
  for (int i = 0; i < resultCount; ++i) {
    const auto& [cohesion, clusterIndex] = ranking[i];
    const auto& cluster = clusters[clusterIndex];

    QVariantList memberList;
    for (const auto memberIndex : cluster.members) {
      memberList.append(QVariantMap{
        {QStringLiteral("itemId"), words[memberIndex].itemId},
        {QStringLiteral("title"),  words[memberIndex].title },
      });
    }

    results.append(QVariantMap{
      {QStringLiteral("name"),       proposeLabel(languageCode, cluster, words)},
      {QStringLiteral("confidence"), confidenceFromCohesion(cohesion)          },
      {QStringLiteral("words"),      memberList                                },
    });
  }

  return results;
}

}  // namespace lexis
