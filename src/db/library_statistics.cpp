#include "library_statistics.hpp"

#include <QDateTime>
#include <QSqlError>
#include <QSqlQuery>

#include <algorithm>
#include <functional>
#include <optional>

#include "library_search.hpp"
#include "schema_migration.hpp"
#include "utils.hpp"

namespace lexis {

namespace {

struct ItemMeta {
  int id = -1;
  int parentId = kRootParentId;
  QString title;
  LibrarySectionType type = LibrarySectionType::kUnknown;
};

using ItemMetaIndex = QHash<int, ItemMeta>;

int scalarCount(QSqlQuery& query) {
  if (!query.exec() || !query.next()) {
    qWarning() << "Statistics query failed:" << query.lastError();
    return 0;
  }
  return query.value(0).toInt();
}

int countItemsAddedSince(const QSqlDatabase& db, const QString& languageCode,
                         const std::function<bool(const QDateTime&)>& predicate) {
  QSqlQuery query(db);
  query.prepare("SELECT creation_time FROM items WHERE language_code = :language_code");
  query.bindValue(":language_code", languageCode);
  if (!query.exec()) {
    qWarning() << "Failed to load item creation times for statistics:" << query.lastError();
    return 0;
  }

  int count = 0;
  while (query.next()) {
    const auto creationTime = parseDateTimeFromDb(query.value(0).toString());
    if (creationTime.isValid() && predicate(creationTime)) {
      ++count;
    }
  }
  return count;
}

ItemMetaIndex loadItemMetaIndex(const QSqlDatabase& db, const QString& languageCode) {
  ItemMetaIndex index;
  QSqlQuery query(db);
  query.prepare(
    "SELECT id, parent_id, title, type FROM items WHERE language_code = :language_code");
  query.bindValue(":language_code", languageCode);
  if (!query.exec()) {
    qWarning() << "Failed to load item metadata for statistics:" << query.lastError();
    return index;
  }

  while (query.next()) {
    const auto id = query.value("id").toInt();
    index.insert(id,
                 ItemMeta{id,
                          query.value("parent_id").isNull() ? kRootParentId
                                                            : query.value("parent_id").toInt(),
                          query.value("title").toString(),
                          static_cast<LibrarySectionType>(query.value("type").toInt())});
  }
  return index;
}

bool isDescendantOf(int itemId, int ancestorId, const ItemMetaIndex& index) {
  int current = itemId;
  while (current != kRootParentId) {
    const auto it = index.constFind(current);
    if (it == index.constEnd()) {
      return false;
    }
    if (it->parentId == ancestorId) {
      return true;
    }
    current = it->parentId;
  }
  return false;
}

QVector<int> wordsInScope(const QSqlDatabase& db, const QString& languageCode, int scopeRootId) {
  QVector<int> wordIds;
  QSqlQuery query(db);

  if (scopeRootId == kRootParentId) {
    query.prepare(
      "SELECT id FROM items "
      "WHERE language_code = :language_code AND type = :word_type");
    query.bindValue(":language_code", languageCode);
    query.bindValue(":word_type", static_cast<int>(LibrarySectionType::kWord));
  } else {
    query.prepare(
      "WITH RECURSIVE subtree(id) AS ("
      "  SELECT :scope_root "
      "  UNION ALL "
      "  SELECT i.id FROM items i "
      "  JOIN subtree s ON i.parent_id = s.id "
      "  WHERE i.language_code = :language_code"
      ") "
      "SELECT id FROM items "
      "WHERE language_code = :language_code "
      "  AND type = :word_type "
      "  AND id IN (SELECT id FROM subtree WHERE id != :scope_root)");
    query.bindValue(":language_code", languageCode);
    query.bindValue(":scope_root", scopeRootId);
    query.bindValue(":word_type", static_cast<int>(LibrarySectionType::kWord));
  }

  if (!query.exec()) {
    qWarning() << "Failed to load scoped words for statistics:" << query.lastError();
    return wordIds;
  }

  while (query.next()) {
    wordIds.append(query.value("id").toInt());
  }
  return wordIds;
}

std::optional<int> directChildSubjectGroupId(int wordId, int scopeRootId,
                                             const ItemMetaIndex& index) {
  if (scopeRootId != kRootParentId && wordId != scopeRootId &&
      !isDescendantOf(wordId, scopeRootId, index)) {
    return std::nullopt;
  }

  int current = wordId;
  while (true) {
    const auto it = index.constFind(current);
    if (it == index.constEnd()) {
      return std::nullopt;
    }

    const int parentId = it->parentId;
    const bool isDirectChildOfScope = scopeRootId == kRootParentId
                                        ? parentId == kRootParentId
                                        : parentId == scopeRootId;
    if (isDirectChildOfScope) {
      if (it->type == LibrarySectionType::kSubjectGroup) {
        return current;
      }
      return std::nullopt;
    }

    if (current == scopeRootId) {
      return std::nullopt;
    }

    current = parentId;
  }
}

QVariantList buildWordsBySubjectGroup(const QSqlDatabase& db, const QString& languageCode,
                                      int scopeRootId, const ItemMetaIndex& index) {
  QHash<int, int> subjectGroupCounts;
  int standaloneWords = 0;

  for (const auto wordId : wordsInScope(db, languageCode, scopeRootId)) {
    const auto subjectGroupId = directChildSubjectGroupId(wordId, scopeRootId, index);
    if (!subjectGroupId.has_value()) {
      ++standaloneWords;
      continue;
    }
    ++subjectGroupCounts[*subjectGroupId];
  }

  struct CategoryEntry {
    QString name;
    int count = 0;
    bool standalone = false;
  };

  QVector<CategoryEntry> entries;
  entries.reserve(subjectGroupCounts.size() + (standaloneWords > 0 ? 1 : 0));
  for (auto it = subjectGroupCounts.constBegin(); it != subjectGroupCounts.constEnd(); ++it) {
    const auto metaIt = index.constFind(it.key());
    if (metaIt == index.constEnd()) {
      continue;
    }
    entries.append({metaIt->title, it.value(), false});
  }
  if (standaloneWords > 0) {
    entries.append({QStringLiteral("Standalone"), standaloneWords, true});
  }

  std::sort(entries.begin(), entries.end(),
            [](const CategoryEntry& left, const CategoryEntry& right) {
              if (left.count != right.count) {
                return left.count > right.count;
              }
              if (left.standalone != right.standalone) {
                return right.standalone;
              }
              return left.name.localeAwareCompare(right.name) < 0;
            });

  QVariantList result;
  for (const auto& entry : entries) {
    result.append(QVariantMap{
      {QStringLiteral("categoryName"), entry.name                   },
      {QStringLiteral("count"),        entry.count                  },
      {QStringLiteral("standalone"),   entry.standalone             }
    });
  }
  return result;
}

}  // namespace

QVariantList LibraryStatistics::scopedWords(const QSqlDatabase& db, const QString& languageCode,
                                            int scopeRootId) {
  QVariantList result;
  if (languageCode.isEmpty()) {
    return result;
  }

  QSqlQuery query(db);
  if (scopeRootId == kRootParentId) {
    query.prepare(
      "SELECT id, title, meaning, cached_translation, color, "
      "CASE WHEN image IS NOT NULL AND length(image) > 0 THEN 1 ELSE 0 END AS has_image "
      "FROM items "
      "WHERE language_code = :language_code AND type = :word_type "
      "ORDER BY title COLLATE NOCASE");
    query.bindValue(":language_code", languageCode);
    query.bindValue(":word_type", static_cast<int>(LibrarySectionType::kWord));
  } else {
    query.prepare(
      "WITH RECURSIVE subtree(id) AS ("
      "  SELECT :scope_root "
      "  UNION ALL "
      "  SELECT i.id FROM items i "
      "  JOIN subtree s ON i.parent_id = s.id "
      "  WHERE i.language_code = :language_code"
      ") "
      "SELECT id, title, meaning, cached_translation, color, "
      "CASE WHEN image IS NOT NULL AND length(image) > 0 THEN 1 ELSE 0 END AS has_image "
      "FROM items "
      "WHERE language_code = :language_code "
      "  AND type = :word_type "
      "  AND id IN (SELECT id FROM subtree WHERE id != :scope_root) "
      "ORDER BY title COLLATE NOCASE");
    query.bindValue(":language_code", languageCode);
    query.bindValue(":scope_root", scopeRootId);
    query.bindValue(":word_type", static_cast<int>(LibrarySectionType::kWord));
  }

  if (!query.exec()) {
    qWarning() << "Failed to load scoped words:" << query.lastError();
    return result;
  }

  while (query.next()) {
    result.append(QVariantMap{
      {QStringLiteral("itemId"),   query.value("id").toInt()              },
      {QStringLiteral("title"),    query.value("title").toString()        },
      {QStringLiteral("meaning"),           query.value("meaning").toString()           },
      {QStringLiteral("cachedTranslation"), query.value("cached_translation").toString()},
      {QStringLiteral("color"),            query.value("color").toString()            },
      {QStringLiteral("hasImage"), query.value("has_image").toInt() != 0  }
    });
  }
  return result;
}

QVariantMap LibraryStatistics::languageStats(const QSqlDatabase& db, const QString& languageCode,
                                             const SectionTypeManager& /*typeManager*/) {
  QVariantMap stats;
  if (languageCode.isEmpty()) {
    return stats;
  }

  QSqlQuery query(db);
  query.prepare("SELECT COUNT(*) FROM items WHERE language_code = :language_code");
  query.bindValue(":language_code", languageCode);
  stats.insert(QStringLiteral("totalItems"), scalarCount(query));

  query.prepare(
    "SELECT COUNT(*) FROM items WHERE language_code = :language_code AND type = :word_type");
  query.bindValue(":language_code", languageCode);
  query.bindValue(":word_type", static_cast<int>(LibrarySectionType::kWord));
  stats.insert(QStringLiteral("wordCount"), scalarCount(query));

  query.prepare(
    "SELECT COUNT(*) FROM items "
    "WHERE language_code = :language_code AND type = :subject_group_type");
  query.bindValue(":language_code", languageCode);
  query.bindValue(":subject_group_type", static_cast<int>(LibrarySectionType::kSubjectGroup));
  stats.insert(QStringLiteral("subjectGroupCount"), scalarCount(query));

  const auto now = QDateTime::currentDateTime();
  const auto currentYear = now.date().year();
  const auto currentMonth = now.date().month();
  const auto last7DaysThreshold = now.addDays(-7);
  const auto last30DaysThreshold = now.addDays(-30);

  stats.insert(QStringLiteral("addedThisMonth"),
                countItemsAddedSince(db, languageCode, [&](const QDateTime& creationTime) {
                  return creationTime.date().year() == currentYear &&
                         creationTime.date().month() == currentMonth;
                }));
  stats.insert(QStringLiteral("addedLast7Days"),
                countItemsAddedSince(db, languageCode, [&](const QDateTime& creationTime) {
                  return creationTime >= last7DaysThreshold;
                }));
  stats.insert(QStringLiteral("addedLast30Days"),
                countItemsAddedSince(db, languageCode, [&](const QDateTime& creationTime) {
                  return creationTime >= last30DaysThreshold;
                }));

  const auto index = loadItemMetaIndex(db, languageCode);
  stats.insert(QStringLiteral("wordsByCategory"),
                buildWordsBySubjectGroup(db, languageCode, kRootParentId, index));

  return stats;
}

QVariantMap LibraryStatistics::itemStats(const QSqlDatabase& db, const QString& languageCode,
                                         int itemId, const SectionTypeManager& typeManager) {
  QVariantMap stats;
  if (languageCode.isEmpty() || itemId <= 0) {
    return stats;
  }

  QSqlQuery query(db);
  query.prepare(
    "SELECT title, creation_time, modification_time, type "
    "FROM items WHERE id = :id AND language_code = :language_code");
  query.bindValue(":id", itemId);
  query.bindValue(":language_code", languageCode);
  if (!query.exec() || !query.next()) {
    qWarning() << "Failed to load item statistics for" << itemId << query.lastError();
    return stats;
  }

  const auto type = typeManager.librarySectionType(query.value("type").toInt());
  stats.insert(QStringLiteral("title"), query.value("title").toString());
  stats.insert(QStringLiteral("type"), static_cast<int>(type));
  stats.insert(QStringLiteral("typeName"), typeManager.librarySectionTypeName(type));
  stats.insert(QStringLiteral("creationTime"),
                parseDateTimeFromDb(query.value("creation_time").toString()));
  stats.insert(QStringLiteral("modificationTime"),
                parseDateTimeFromDb(query.value("modification_time").toString()));

  const auto index = loadItemMetaIndex(db, languageCode);
  stats.insert(QStringLiteral("breadcrumb"), LibrarySearch::parentPath(
                                               LibrarySearch::loadItemIndex(db, languageCode),
                                               itemId));

  query.prepare(
    "SELECT COUNT(*) FROM items "
    "WHERE language_code = :language_code AND parent_id = :parent_id");
  query.bindValue(":language_code", languageCode);
  query.bindValue(":parent_id", itemId);
  stats.insert(QStringLiteral("directChildren"), scalarCount(query));

  query.prepare(
    "WITH RECURSIVE descendants(id) AS ("
    "  SELECT id FROM items "
    "  WHERE language_code = :language_code AND parent_id = :parent_id "
    "  UNION ALL "
    "  SELECT i.id FROM items i "
    "  JOIN descendants d ON i.parent_id = d.id "
    "  WHERE i.language_code = :language_code"
    ") "
    "SELECT COUNT(*) FROM descendants");
  query.bindValue(":language_code", languageCode);
  query.bindValue(":parent_id", itemId);
  stats.insert(QStringLiteral("descendants"), scalarCount(query));

  if (type != LibrarySectionType::kWord) {
    stats.insert(QStringLiteral("wordsByCategory"),
                  buildWordsBySubjectGroup(db, languageCode, itemId, index));
  }

  return stats;
}

}  // namespace lexis
