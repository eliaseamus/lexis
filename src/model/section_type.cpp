#include "section_type.hpp"

#include <QHash>

#include "utils.hpp"

namespace lexis {

QHash<LibrarySectionType, QString> SectionTypeManager::librarySectionTypeNames() {
  using enum LibrarySectionType;
  static const QHash<LibrarySectionType, QString> names = {
    {kSubjectGroup, tr("Subject group")},
    {kBook,         tr("Book")},
    {kArticle,      tr("Article")},
    {kMovie,        tr("Movie")},
    {kSeries,       tr("Series")},
    {kAlbum,        tr("Album")},
    {kSong,         tr("Song")}
  };
  return names;
}

QStringList SectionTypeManager::librarySectionNames() {
  using enum LibrarySectionType;

  QStringList res;
  for (const auto e : enumRange(kSubjectGroup, kSong)) {
    res << librarySectionTypeName(e);
  }

  return res;
}

QString SectionTypeManager::librarySectionTypeName(LibrarySectionType type) {
  static const auto typeNames = librarySectionTypeNames();

  if (typeNames.find(type) == typeNames.end()) {
    return QString("Unknown section: %1").arg(static_cast<int>(type));
  }

  return typeNames[type];
}

LibrarySectionType SectionTypeManager::librarySectionType(const QString& name) {
  static const auto typeNames = librarySectionTypeNames();
  return typeNames.key(name, LibrarySectionType::kUnknown);
}

LibrarySectionType SectionTypeManager::librarySectionType(int type) {
  using enum LibrarySectionType;
  int bottom = std::to_underlying(kSubjectGroup);
  int top = std::to_underlying(kSong);
  if (type < bottom || type > top) {
    return kUnknown;
  }
  return static_cast<LibrarySectionType>(type);
}

}
