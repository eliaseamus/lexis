#include "library_group_suggestion.hpp"

#include <QSet>
#include <QSqlError>
#include <QSqlQuery>

#include <algorithm>

#include "library_search.hpp"
#include "schema_migration.hpp"
#include "section_type.hpp"

namespace lexis {

namespace {

struct ScoredGroup {
  int groupId = -1;
  QString groupName;
  QString breadcrumb;
  double score = 0.0;
  int confidence = 0;
};

QStringList tokenize(const QString& text) {
  QStringList tokens;
  QString current;
  for (const auto character : text.toCaseFolded()) {
    if (character.isLetterOrNumber()) {
      current.append(character);
      continue;
    }
    if (!current.isEmpty()) {
      tokens.append(current);
      current.clear();
    }
  }
  if (!current.isEmpty()) {
    tokens.append(current);
  }
  return tokens;
}

double jaccardSimilarity(const QStringList& leftTokens, const QStringList& rightTokens) {
  if (leftTokens.isEmpty() || rightTokens.isEmpty()) {
    return 0.0;
  }

  QSet<QString> leftSet(leftTokens.begin(), leftTokens.end());
  QSet<QString> rightSet(rightTokens.begin(), rightTokens.end());
  int intersection = 0;
  for (const auto& token : leftSet) {
    if (rightSet.contains(token)) {
      ++intersection;
    }
  }

  const int unionSize = leftSet.size() + rightSet.size() - intersection;
  if (unionSize <= 0) {
    return 0.0;
  }
  return static_cast<double>(intersection) / unionSize;
}

double substringSimilarity(const QString& left, const QString& right) {
  const auto normalizedLeft = left.trimmed().toCaseFolded();
  const auto normalizedRight = right.trimmed().toCaseFolded();
  if (normalizedLeft.isEmpty() || normalizedRight.isEmpty()) {
    return 0.0;
  }
  if (normalizedLeft == normalizedRight) {
    return 1.0;
  }
  if (normalizedLeft.contains(normalizedRight) || normalizedRight.contains(normalizedLeft)) {
    return 0.75;
  }
  return 0.0;
}

double textSimilarity(const QString& left, const QString& right) {
  const auto jaccard = jaccardSimilarity(tokenize(left), tokenize(right));
  const auto substring = substringSimilarity(left, right);
  return jaccard >= substring ? jaccard : substring;
}

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

double scoreGroup(const QString& wordTitle, const QString& meaning, const QString& groupTitle,
                  const QVector<QPair<QString, QString>>& wordsInGroup) {
  double score = 0.0;
  score += textSimilarity(wordTitle, groupTitle) * 0.35;

  if (!wordsInGroup.isEmpty()) {
    double bestWordScore = 0.0;
    double totalWordScore = 0.0;
    for (const auto& [existingTitle, existingMeaning] : wordsInGroup) {
      const auto wordScore = textSimilarity(wordTitle, existingTitle);
      bestWordScore = wordScore > bestWordScore ? wordScore : bestWordScore;
      totalWordScore += wordScore;
    }
    score += bestWordScore * 0.35;
    score += (totalWordScore / wordsInGroup.size()) * 0.10;
  }

  if (!meaning.isEmpty()) {
    score += textSimilarity(meaning, groupTitle) * 0.10;
    double bestMeaningScore = 0.0;
    for (const auto& [existingTitle, existingMeaning] : wordsInGroup) {
      Q_UNUSED(existingTitle);
      if (existingMeaning.isEmpty()) {
        continue;
      }
      const auto meaningScore = textSimilarity(meaning, existingMeaning);
      bestMeaningScore = meaningScore > bestMeaningScore ? meaningScore : bestMeaningScore;
    }
    score += bestMeaningScore * 0.35;
  }

  return score;
}

}  // namespace

QVariantList LibraryGroupSuggestion::suggestSubjectGroups(const QSqlDatabase& db,
                                                          const QString& languageCode,
                                                          const QString& wordTitle,
                                                          const QString& meaning,
                                                          int excludeItemId,
                                                          int currentParentId, int limit) {
  QVariantList results;
  const auto trimmedTitle = wordTitle.trimmed();
  if (languageCode.isEmpty() || trimmedTitle.isEmpty() || limit <= 0) {
    return results;
  }

  QSqlQuery query(db);
  query.prepare(
    "SELECT id, parent_id, title, type, meaning FROM items WHERE language_code = :language_code");
  query.bindValue(":language_code", languageCode);
  if (!query.exec()) {
    qWarning() << "Failed to load items for group suggestions:" << query.lastError();
    return results;
  }

  QHash<int, QString> titles;
  QHash<int, QString> meanings;
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
    meanings.insert(id, query.value("meaning").toString());
    typeById.insert(id, type);
    childrenByParent[parentId].append(id);

    if (type == LibrarySectionType::kSubjectGroup) {
      subjectGroupIds.append(id);
    }
  }

  if (subjectGroupIds.isEmpty()) {
    return results;
  }

  const auto index = LibrarySearch::loadItemIndex(db, languageCode);
  QVector<ScoredGroup> scoredGroups;
  scoredGroups.reserve(subjectGroupIds.size());

  for (const auto groupId : subjectGroupIds) {
    if (groupId == currentParentId) {
      continue;
    }

    QVector<QPair<QString, QString>> wordsInGroup;
    for (const auto wordId : descendantWordIds(groupId, childrenByParent, typeById)) {
      wordsInGroup.append({titles.value(wordId), meanings.value(wordId)});
    }

    const auto groupTitle = titles.value(groupId);
    const auto score = scoreGroup(trimmedTitle, meaning.trimmed(), groupTitle, wordsInGroup);
    if (score < 0.12) {
      continue;
    }

    scoredGroups.append({groupId,
                         groupTitle,
                         LibrarySearch::breadcrumb(index, groupId),
                         score,
                         0});
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

  const auto topScore = scoredGroups.front().score;
  const int resultCount = limit < scoredGroups.size() ? limit : scoredGroups.size();
  for (int i = 0; i < resultCount; ++i) {
    auto entry = scoredGroups[i];
    entry.confidence = topScore > 0.0 ? qRound(100.0 * entry.score / topScore) : 0;
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
