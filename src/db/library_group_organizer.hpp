#pragma once

#include <QSqlDatabase>
#include <QVariantList>

namespace lexis {

// Clusters loose words (direct children of a folder) by embedding similarity
// and proposes new subject groups with an auto-picked name. Requires the
// embedding database; returns an empty list when it is unavailable.
class LibraryGroupOrganizer {
 public:
  static QVariantList proposeGroups(const QSqlDatabase& db, const QString& languageCode,
                                    int scopeRootId, int minClusterSize = 2,
                                    int maxProposals = 4);
};

}  // namespace lexis
