#include "section_type.hpp"

#include <QHash>

namespace lexis {

static QHash<LibrarySectionType, QString> librarySectionTypeNames() {
  using enum LibrarySectionType;
  static const QHash<LibrarySectionType, QString> names = {
    {kSubjectGroup, "Subject group"},
    {kBook,         "Book"},
    {kArticle,      "Article"},
    {kMovie,        "Movie"},
    {kSeries,       "Series"},
    {kAlbum,        "Album"},
    {kSong,         "Song"}
  };
  return names;
}

QStringList SectionTypeManager::librarySectionNames() {
  static const auto typeNames = librarySectionTypeNames();
  auto res = QStringList{typeNames.values()};
  res.sort();
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

}
