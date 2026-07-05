#pragma once

#include <QSqlDatabase>
#include <QVariantList>

namespace lexis {

class LibraryGroupSuggestion {
 public:
  static QVariantList suggestSubjectGroups(const QSqlDatabase& db, const QString& languageCode,
                                           const QString& wordTitle, const QString& meaning,
                                           const QString& dictionarySummary, int excludeItemId,
                                           int currentParentId, int limit = 3);
  // Maps an internal match score to an absolute 0-100 confidence.
  static int confidenceFromScore(double score);
};

}  // namespace lexis
