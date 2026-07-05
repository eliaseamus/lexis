#include "library_group_suggestion.hpp"

#include <QSqlError>
#include <QSqlQuery>

#include <algorithm>

#include "dictionary_summary.hpp"
#include "library_search.hpp"
#include "schema_migration.hpp"
#include "section_type.hpp"
#include "text_embedding.hpp"
#include "text_similarity.hpp"

namespace lexis {

namespace {

struct ScoredGroup {
  int groupId = -1;
  QString groupName;
  QString breadcrumb;
  double score = 0.0;
  int confidence = 0;
};

QVector<int> descendantWordIds(int groupId, const QHash<int, QVector<int>>& childrenByParent,
                               const QHash<int, LibrarySectionType>& typeById) {
  QVector<int> wordIds;
  QVector<int> stack{groupId};

  while (!stack.isEmpty()) {
    const auto itemId = stack.takeLast();
    const auto childIt = childrenByParent.constFind(itemId);
    if (childIt == childrenByParent.constEnd()) {
      continue;
    }

    for (const auto childId : *childIt) {
      const auto typeIt = typeById.constFind(childId);
      if (typeIt == typeById.constEnd()) {
        continue;
      }
      if (*typeIt == LibrarySectionType::kWord) {
        wordIds.append(childId);
      } else {
        stack.append(childId);
      }
    }
  }

  return wordIds;
}

double lexicalGroupScore(const QString& wordTitle, const QString& semanticContext,
                         const QString& groupTitle,
                         const QVector<QPair<QString, QString>>& wordsInGroup) {
  double score = 0.0;
  score += textSimilarity(wordTitle, groupTitle) * 0.35;

  if (!wordsInGroup.isEmpty()) {
    double bestWordScore = 0.0;
    double totalWordScore = 0.0;
    for (const auto& [existingTitle, existingSemanticContext] : wordsInGroup) {
      const auto wordScore = textSimilarity(wordTitle, existingTitle);
      bestWordScore = wordScore > bestWordScore ? wordScore : bestWordScore;
      totalWordScore += wordScore;
    }
    score += bestWordScore * 0.35;
    score += (totalWordScore / wordsInGroup.size()) * 0.10;
  }

  if (!semanticContext.isEmpty()) {
    score += textSimilarity(semanticContext, groupTitle) * 0.10;
    double bestSemanticScore = 0.0;
    for (const auto& [existingTitle, existingSemanticContext] : wordsInGroup) {
      Q_UNUSED(existingTitle);
      if (existingSemanticContext.isEmpty()) {
        continue;
      }
      const auto semanticScore = textSimilarity(semanticContext, existingSemanticContext);
      bestSemanticScore = semanticScore > bestSemanticScore ? semanticScore : bestSemanticScore;
    }
    score += bestSemanticScore * 0.35;
  }

  return score;
}

// Raw cosine similarities between arbitrary related words sit well below 1.0,
// and unrelated words rarely drop to exactly 0. Rescale to strip the noise
// floor and stretch strong relations toward 1.0.
double rescaleCosine(double similarity) {
  const auto rescaled = (similarity - 0.30) / 0.70;
  if (rescaled < 0.0) {
    return 0.0;
  }
  return rescaled > 1.0 ? 1.0 : rescaled;
}

double embeddingGroupScore(const EmbeddingVector& wordVector,
                           const EmbeddingVector& contextVector,
                           const EmbeddingVector& groupTitleVector,
                           const QVector<EmbeddingVector>& memberVectors) {
  const auto& targetVector = wordVector.isEmpty() ? contextVector : wordVector;
  if (targetVector.isEmpty()) {
    return 0.0;
  }

  auto centroid = TextEmbedding::meanVector(memberVectors);
  TextEmbedding::normalize(centroid);

  double score = 0.0;
  score += rescaleCosine(TextEmbedding::similarity(targetVector, groupTitleVector)) * 0.35;
  score += rescaleCosine(TextEmbedding::similarity(targetVector, centroid)) * 0.45;

  if (!contextVector.isEmpty()) {
    const auto contextSimilarity =
      std::max(TextEmbedding::similarity(contextVector, groupTitleVector),
               TextEmbedding::similarity(contextVector, centroid));
    score += rescaleCosine(contextSimilarity) * 0.20;
  }

  return score;
}

}  // namespace

int LibraryGroupSuggestion::confidenceFromScore(double score) {
  // A score of ~0.7 already means a very strong match with either scoring
  // strategy, so treat it as full confidence.
  const auto normalized = score / 0.7;
  const auto clamped = normalized < 0.0 ? 0.0 : (normalized > 1.0 ? 1.0 : normalized);
  return qRound(100.0 * clamped);
}

QVariantList LibraryGroupSuggestion::suggestSubjectGroups(
  const QSqlDatabase& db, const QString& languageCode, const QString& wordTitle,
  const QString& meaning, const QString& dictionarySummary, int excludeItemId,
  int currentParentId, int limit) {
  QVariantList results;
  const auto trimmedTitle = wordTitle.trimmed();
  if (languageCode.isEmpty() || trimmedTitle.isEmpty() || limit <= 0) {
    return results;
  }

  const auto targetSemanticContext =
    combineSemanticContext(meaning.trimmed(), dictionarySummary.trimmed());

  QSqlQuery query(db);
  query.prepare(
    "SELECT id, parent_id, title, type, meaning, dictionary_summary, cached_translation "
    "FROM items WHERE language_code = :language_code");
  query.bindValue(":language_code", languageCode);
  if (!query.exec()) {
    qWarning() << "Failed to load items for group suggestions:" << query.lastError();
    return results;
  }

  QHash<int, QString> titles;
  QHash<int, QString> semanticContexts;
  QHash<int, LibrarySectionType> typeById;
  QHash<int, QVector<int>> childrenByParent;
  QVector<int> subjectGroupIds;

  while (query.next()) {
    const auto id = query.value("id").toInt();
    if (id == excludeItemId) {
      continue;
    }

    const auto type = static_cast<LibrarySectionType>(query.value("type").toInt());
    const auto parentId = query.value("parent_id").isNull() ? kRootParentId
                                                            : query.value("parent_id").toInt();

    titles.insert(id, query.value("title").toString());
    semanticContexts.insert(id, combineSemanticContext(query.value("meaning").toString(),
                                                         query.value("dictionary_summary").toString(),
                                                         query.value("cached_translation").toString()));
    typeById.insert(id, type);
    childrenByParent[parentId].append(id);

    if (type == LibrarySectionType::kSubjectGroup) {
      subjectGroupIds.append(id);
    }
  }

  if (subjectGroupIds.isEmpty()) {
    return results;
  }

  const bool useEmbeddings = EmbeddingLookup::isOpen();
  const auto targetVector =
    useEmbeddings ? TextEmbedding::embedText(languageCode, trimmedTitle) : EmbeddingVector{};
  const auto contextVector = useEmbeddings
                               ? TextEmbedding::embedText(languageCode, targetSemanticContext)
                               : EmbeddingVector{};

  QHash<int, EmbeddingVector> memberVectorCache;
  const auto memberVector = [&](int wordId) -> const EmbeddingVector& {
    auto it = memberVectorCache.find(wordId);
    if (it == memberVectorCache.end()) {
      it = memberVectorCache.insert(wordId,
                                    TextEmbedding::embedText(languageCode, titles.value(wordId)));
    }
    return *it;
  };

  const auto index = LibrarySearch::loadItemIndex(db, languageCode);
  QVector<ScoredGroup> scoredGroups;
  scoredGroups.reserve(subjectGroupIds.size());

  for (const auto groupId : subjectGroupIds) {
    if (groupId == currentParentId) {
      continue;
    }

    const auto wordIds = descendantWordIds(groupId, childrenByParent, typeById);
    QVector<QPair<QString, QString>> wordsInGroup;
    QVector<EmbeddingVector> memberVectors;
    for (const auto wordId : wordIds) {
      wordsInGroup.append({titles.value(wordId), semanticContexts.value(wordId)});
      if (useEmbeddings) {
        const auto& vector = memberVector(wordId);
        if (!vector.isEmpty()) {
          memberVectors.append(vector);
        }
      }
    }

    const auto groupTitle = titles.value(groupId);
    auto score = lexicalGroupScore(trimmedTitle, targetSemanticContext, groupTitle, wordsInGroup);
    if (useEmbeddings) {
      const auto groupTitleVector = TextEmbedding::embedText(languageCode, groupTitle);
      const auto semanticScore =
        embeddingGroupScore(targetVector, contextVector, groupTitleVector, memberVectors);
      score = std::max(score, semanticScore);
    }
    if (score < 0.12) {
      continue;
    }

    scoredGroups.append({groupId,
                         groupTitle,
                         LibrarySearch::breadcrumb(index, groupId),
                         score,
                         confidenceFromScore(score)});
  }

  if (scoredGroups.isEmpty()) {
    return results;
  }

  std::sort(scoredGroups.begin(), scoredGroups.end(),
            [](const ScoredGroup& left, const ScoredGroup& right) {
              if (left.score != right.score) {
                return left.score > right.score;
              }
              return left.groupName.localeAwareCompare(right.groupName) < 0;
            });

  const int resultCount = limit < scoredGroups.size() ? limit : scoredGroups.size();
  for (int i = 0; i < resultCount; ++i) {
    const auto& entry = scoredGroups[i];
    results.append(QVariantMap{
      {QStringLiteral("groupId"),    entry.groupId    },
      {QStringLiteral("groupName"),  entry.groupName  },
      {QStringLiteral("breadcrumb"), entry.breadcrumb },
      {QStringLiteral("confidence"), entry.confidence },
    });
  }

  return results;
}

}  // namespace lexis
