#include "library_search.hpp"

#include <QSqlError>
#include <QSqlQuery>

#include "utils.hpp"

namespace lexis {

namespace {

constexpr int kWordType = std::to_underlying(LibrarySectionType::kWord);

int parentIdFromVariant(const QVariant& value) {
  if (!value.isValid() || value.isNull()) {
    return kRootParentId;
  }
  return value.toInt();
}

QStringList tokenizeQuery(const QString& text) {
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

QString buildFtsQuery(const QString& queryText) {
  QStringList terms;
  for (const auto& token : tokenizeQuery(queryText)) {
    if (token.size() < 2) {
      continue;
    }
    QString escaped = token;
    escaped.replace('"', QStringLiteral("\"\""));
    terms.append(QStringLiteral("\"%1\"*").arg(escaped));
  }
  return terms.join(QLatin1Char(' '));
}

QString firstMatchingLine(const QString& query, const QString& text) {
  if (text.trimmed().isEmpty()) {
    return {};
  }

  for (const auto& line : text.split(QLatin1Char('\n'))) {
    if (line.contains(query, Qt::CaseInsensitive)) {
      return line.trimmed();
    }
  }
  return {};
}

QString matchedSnippet(const QString& query, const QString& title, const QString& meaning,
                       const QString& dictionarySummary, const QString& cachedTranslation) {
  if (title.contains(query, Qt::CaseInsensitive)) {
    return title;
  }

  if (const auto summaryLine = firstMatchingLine(query, dictionarySummary); !summaryLine.isEmpty()) {
    return summaryLine;
  }

  if (const auto translationLine = firstMatchingLine(query, cachedTranslation);
      !translationLine.isEmpty()) {
    return translationLine;
  }

  const auto plainMeaning = stripHtmlPlainText(meaning);
  if (plainMeaning.contains(query, Qt::CaseInsensitive)) {
    return plainMeaning;
  }

  if (!dictionarySummary.trimmed().isEmpty()) {
    const auto lines = dictionarySummary.split(QLatin1Char('\n'));
    if (!lines.isEmpty()) {
      return lines.first().trimmed();
    }
  }

  return plainMeaning;
}

QVariantMap makeResultMap(const QSqlQuery& row, const LibrarySearch::ItemIndex& index,
                          const SectionTypeManager& typeManager, const QString& snippetQuery) {
  const auto id = row.value("id").toInt();
  const auto title = row.value("title").toString();
  const auto meaning = row.value("meaning").toString();
  const auto dictionarySummary = row.value("dictionary_summary").toString();
  const auto cachedTranslation = row.value("cached_translation").toString();
  const auto type = typeManager.librarySectionType(row.value("type").toInt());
  const auto image = row.value("image").toByteArray();

  QVariantMap result;
  result.insert(QStringLiteral("itemId"), id);
  result.insert(QStringLiteral("parentId"), parentIdFromVariant(row.value("parent_id")));
  result.insert(QStringLiteral("title"), title);
  result.insert(QStringLiteral("meaning"), meaning);
  result.insert(QStringLiteral("snippet"),
                matchedSnippet(snippetQuery, title, meaning, dictionarySummary, cachedTranslation));
  result.insert(QStringLiteral("type"), typeManager.librarySectionTypeName(type));
  result.insert(QStringLiteral("typeEnum"), static_cast<int>(type));
  result.insert(QStringLiteral("itemColor"), row.value("color").toString());
  result.insert(QStringLiteral("hasImage"), !image.isEmpty());
  const auto frequencyTier = row.value("frequency_tier").toString();
  if (!frequencyTier.isEmpty()) {
    result.insert(QStringLiteral("frequencyTier"), frequencyTier);
  }
  result.insert(QStringLiteral("breadcrumb"), LibrarySearch::breadcrumb(index, id));
  result.insert(QStringLiteral("parentPath"), LibrarySearch::parentPath(index, id));
  return result;
}

bool ftsIndexAvailable(QSqlDatabase& db) {
  QSqlQuery query(db);
  query.prepare("SELECT 1 FROM sqlite_master WHERE type = 'table' AND name = :name");
  query.bindValue(":name", QStringLiteral("items_fts"));
  return query.exec() && query.next();
}

QVariantList searchWithFts(QSqlDatabase& db, const QString& languageCode, const QString& ftsQuery,
                           const SectionTypeManager& typeManager, const QString& snippetQuery,
                           int limit) {
  QVariantList results;

  QSqlQuery sqlQuery(db);
  sqlQuery.prepare(
    "SELECT i.id, i.parent_id, i.title, i.meaning, i.type, i.color, i.image, i.frequency_tier, "
    "i.dictionary_summary, i.cached_translation "
    "FROM items_fts fts "
    "JOIN items i ON i.id = fts.item_id "
    "WHERE fts.language_code = :language_code "
    "  AND fts MATCH :pattern "
    "ORDER BY rank "
    "LIMIT :limit");
  sqlQuery.bindValue(":language_code", languageCode);
  sqlQuery.bindValue(":pattern", ftsQuery);
  sqlQuery.bindValue(":limit", limit);

  if (!sqlQuery.exec()) {
    qWarning() << "FTS search failed:" << sqlQuery.lastError();
    return results;
  }

  const auto index = LibrarySearch::loadItemIndex(db, languageCode);
  while (sqlQuery.next()) {
    results.append(makeResultMap(sqlQuery, index, typeManager, snippetQuery));
  }
  return results;
}

QVariantList searchWithLike(QSqlDatabase& db, const QString& languageCode, const QString& trimmed,
                            const SectionTypeManager& typeManager, int limit) {
  QVariantList results;

  QSqlQuery sqlQuery(db);
  sqlQuery.prepare(
    "SELECT id, parent_id, title, meaning, type, color, image, frequency_tier, "
    "dictionary_summary, cached_translation "
    "FROM items "
    "WHERE language_code = :language_code "
    "  AND (title LIKE :pattern COLLATE NOCASE "
    "       OR IFNULL(meaning, '') LIKE :pattern COLLATE NOCASE "
    "       OR IFNULL(dictionary_summary, '') LIKE :pattern COLLATE NOCASE "
    "       OR IFNULL(cached_translation, '') LIKE :pattern COLLATE NOCASE) "
    "ORDER BY CASE "
    "  WHEN title LIKE :pattern COLLATE NOCASE THEN 0 "
    "  WHEN IFNULL(dictionary_summary, '') LIKE :pattern COLLATE NOCASE THEN 1 "
    "  WHEN IFNULL(cached_translation, '') LIKE :pattern COLLATE NOCASE THEN 2 "
    "  ELSE 3 END, title "
    "LIMIT :limit");
  sqlQuery.bindValue(":language_code", languageCode);
  sqlQuery.bindValue(":pattern", QStringLiteral("%%1%").arg(trimmed));
  sqlQuery.bindValue(":limit", limit);

  if (!sqlQuery.exec()) {
    qWarning() << "Search failed:" << sqlQuery.lastError();
    return results;
  }

  const auto index = LibrarySearch::loadItemIndex(db, languageCode);
  while (sqlQuery.next()) {
    results.append(makeResultMap(sqlQuery, index, typeManager, trimmed));
  }
  return results;
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

  QSqlDatabase mutableDb = db;
  if (ftsIndexAvailable(mutableDb)) {
    const auto ftsQuery = buildFtsQuery(trimmed);
    if (!ftsQuery.isEmpty()) {
      results = searchWithFts(mutableDb, languageCode, ftsQuery, typeManager, trimmed, limit);
      if (!results.isEmpty()) {
        return results;
      }
    }
  }

  return searchWithLike(mutableDb, languageCode, trimmed, typeManager, limit);
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
    "SELECT id, parent_id, title, meaning, type, color, image, frequency_tier, "
    "dictionary_summary, cached_translation "
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
