#pragma once

#include <QSqlDatabase>
#include <QVariantList>

#include "schema_migration.hpp"
#include "section_type.hpp"

namespace lexis {

class LibrarySearch {
 public:
  struct ItemRef {
    int id = -1;
    int parentId = kRootParentId;
    QString title;
  };

  using ItemIndex = QHash<int, ItemRef>;

  static ItemIndex loadItemIndex(const QSqlDatabase& db, const QString& languageCode);
  static QString breadcrumb(const ItemIndex& index, int itemId);
  static QString parentPath(const ItemIndex& index, int itemId);
  static QVariantList ancestorPath(const ItemIndex& index, int itemId);
  static QVariantList search(const QSqlDatabase& db, const QString& languageCode, const QString& query,
                             const SectionTypeManager& typeManager, int limit = 100);
  static QVariantList findByTitle(const QSqlDatabase& db, const QString& languageCode,
                                  const QString& title, const SectionTypeManager& typeManager,
                                  int excludeItemId = -1);
  static QVariantList findAllDuplicates(const QSqlDatabase& db, const QString& languageCode,
                                        const SectionTypeManager& typeManager);
};

}  // namespace lexis
