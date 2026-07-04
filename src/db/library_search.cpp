#include "library_search.hpp"

#include <QSqlError>
#include <QSqlQuery>

namespace lexis {

namespace {

constexpr int kWordType = std::to_underlying(LibrarySectionType::kWord);

int parentIdFromVariant(const QVariant& value) {
  if (!value.isValid() || value.isNull()) {
    return kRootParentId;
  }
  return value.toInt();
}

QString matchedSnippet(const QString& query, const QString& title, const QString& meaning) {
  if (title.contains(query, Qt::CaseInsensitive)) {
    return title;
  }
  return meaning;
}

QVariantMap makeResultMap(const QSqlQuery& row, const LibrarySearch::ItemIndex& index,
                          const SectionTypeManager& typeManager, const QString& snippetQuery) {
  const auto id = row.value("id").toInt();
  const auto title = row.value("title").toString();
  const auto meaning = row.value("meaning").toString();
  const auto type = typeManager.librarySectionType(row.value("type").toInt());
  const auto image = row.value("image").toByteArray();

  QVariantMap result;
  result.insert(QStringLiteral("itemId"), id);
  result.insert(QStringLiteral("parentId"), parentIdFromVariant(row.value("parent_id")));
  result.insert(QStringLiteral("title"), title);
  result.insert(QStringLiteral("meaning"), meaning);
  result.insert(QStringLiteral("snippet"), matchedSnippet(snippetQuery, title, meaning));
  result.insert(QStringLiteral("type"), typeManager.librarySectionTypeName(type));
  result.insert(QStringLiteral("typeEnum"), static_cast<int>(type));
  result.insert(QStringLiteral("itemColor"), row.value("color").toString());
  result.insert(QStringLiteral("hasImage"), !image.isEmpty());
  result.insert(QStringLiteral("breadcrumb"), LibrarySearch::breadcrumb(index, id));
  result.insert(QStringLiteral("parentPath"), LibrarySearch::parentPath(index, id));
  return result;
}

}  // namespace

LibrarySearch::ItemIndex LibrarySearch::loadItemIndex(const QSqlDatabase& db,
                                                      const QString& languageCode) {
  ItemIndex index;
  QSqlQuery query(db);
  query.prepare("SELECT id, parent_id, title FROM items WHERE language_code = :language_code");
  query.bindValue(":language_code", languageCode);
  if (!query.exec()) {
    qWarning() << "Failed to load search index:" << query.lastError();
    return index;
  }

  while (query.next()) {
    const auto id = query.value("id").toInt();
    index.insert(id,
                 ItemRef{id, parentIdFromVariant(query.value("parent_id")),
                         query.value("title").toString()});
  }
  return index;
}

QString LibrarySearch::breadcrumb(const ItemIndex& index, int itemId) {
  QStringList parts;
  int currentId = itemId;
  while (currentId != kRootParentId) {
    const auto it = index.constFind(currentId);
    if (it == index.constEnd()) {
      break;
    }
    parts.prepend(it->title);
    currentId = it->parentId;
  }
  return parts.join(QStringLiteral(" \u203a "));
}

QString LibrarySearch::parentPath(const ItemIndex& index, int itemId) {
  const auto it = index.constFind(itemId);
  if (it == index.constEnd() || it->parentId == kRootParentId) {
    return {};
  }
  return breadcrumb(index, it->parentId);
}

QVariantList LibrarySearch::ancestorPath(const ItemIndex& index, int itemId) {
  QVariantList path;
  int currentId = itemId;
  while (currentId != kRootParentId) {
    const auto it = index.constFind(currentId);
    if (it == index.constEnd()) {
      break;
    }
    path.prepend(QVariantMap{
      {"id",    it->id   },
      {"title", it->title}
    });
    currentId = it->parentId;
  }
  return path;
}

QVariantList LibrarySearch::search(const QSqlDatabase& db, const QString& languageCode,
                                   const QString& queryText, const SectionTypeManager& typeManager,
                                   int limit) {
  QVariantList results;
  const auto trimmed = queryText.trimmed();
  if (languageCode.isEmpty() || trimmed.isEmpty()) {
    return results;
  }

  QSqlQuery sqlQuery(db);
  sqlQuery.prepare(
    "SELECT id, parent_id, title, meaning, type, color, image "
    "FROM items "
    "WHERE language_code = :language_code "
    "  AND (title LIKE :pattern COLLATE NOCASE "
    "       OR IFNULL(meaning, '') LIKE :pattern COLLATE NOCASE) "
    "ORDER BY title "
    "LIMIT :limit");
  sqlQuery.bindValue(":language_code", languageCode);
  sqlQuery.bindValue(":pattern", QStringLiteral("%%1%").arg(trimmed));
  sqlQuery.bindValue(":limit", limit);

  if (!sqlQuery.exec()) {
    qWarning() << "Search failed:" << sqlQuery.lastError();
    return results;
  }

  const auto index = loadItemIndex(db, languageCode);
  while (sqlQuery.next()) {
    results.append(makeResultMap(sqlQuery, index, typeManager, trimmed));
  }
  return results;
}

QVariantList LibrarySearch::findByTitle(const QSqlDatabase& db, const QString& languageCode,
                                        const QString& title, const SectionTypeManager& typeManager,
                                        int excludeItemId) {
  QVariantList results;
  const auto trimmed = title.trimmed();
  if (languageCode.isEmpty() || trimmed.isEmpty()) {
    return results;
  }

  QSqlQuery sqlQuery(db);
  sqlQuery.prepare(
    "SELECT id, parent_id, title, meaning, type, color, image "
    "FROM items "
    "WHERE language_code = :language_code "
    "  AND type = :word_type "
    "  AND title = :title COLLATE NOCASE "
    "  AND (:exclude_id < 0 OR id != :exclude_id) "
    "ORDER BY title");
  sqlQuery.bindValue(":language_code", languageCode);
  sqlQuery.bindValue(":word_type", kWordType);
  sqlQuery.bindValue(":title", trimmed);
  sqlQuery.bindValue(":exclude_id", excludeItemId);

  if (!sqlQuery.exec()) {
    qWarning() << "Duplicate lookup failed:" << sqlQuery.lastError();
    return results;
  }

  const auto index = loadItemIndex(db, languageCode);
  while (sqlQuery.next()) {
    results.append(makeResultMap(sqlQuery, index, typeManager, trimmed));
  }
  return results;
}

}  // namespace lexis
