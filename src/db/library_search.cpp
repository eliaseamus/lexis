#include "library_search.hpp"

#include <QSqlError>
#include <QSqlQuery>

namespace lexis {

namespace {

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
    "SELECT id, parent_id, title, meaning, type, color "
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
    const auto id = sqlQuery.value("id").toInt();
    const auto title = sqlQuery.value("title").toString();
    const auto meaning = sqlQuery.value("meaning").toString();
    const auto type = typeManager.librarySectionType(sqlQuery.value("type").toInt());

    QVariantMap result;
    result.insert(QStringLiteral("itemId"), id);
    result.insert(QStringLiteral("parentId"), parentIdFromVariant(sqlQuery.value("parent_id")));
    result.insert(QStringLiteral("title"), title);
    result.insert(QStringLiteral("meaning"), meaning);
    result.insert(QStringLiteral("snippet"), matchedSnippet(trimmed, title, meaning));
    result.insert(QStringLiteral("type"), typeManager.librarySectionTypeName(type));
    result.insert(QStringLiteral("typeEnum"), static_cast<int>(type));
    result.insert(QStringLiteral("itemColor"), sqlQuery.value("color").toString());
    result.insert(QStringLiteral("breadcrumb"), breadcrumb(index, id));
    results.append(result);
  }
  return results;
}

}  // namespace lexis
