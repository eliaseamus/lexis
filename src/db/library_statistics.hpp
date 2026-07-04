#pragma once

#include <QSqlDatabase>
#include <QVariantMap>

#include "section_type.hpp"

namespace lexis {

class LibraryStatistics {
 public:
  static QVariantMap languageStats(const QSqlDatabase& db, const QString& languageCode,
                                 const SectionTypeManager& typeManager);
  static QVariantMap itemStats(const QSqlDatabase& db, const QString& languageCode, int itemId,
                               const SectionTypeManager& typeManager);
};

}  // namespace lexis
