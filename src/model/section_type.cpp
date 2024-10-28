#include "section_type.hpp"

#include <QHash>

#include "utils.hpp"

namespace lexis {

SectionTypeManager::SectionTypeManager(QObject* parent) :
  QObject(parent)
{
  using enum LibrarySectionType;
  _typeNames = {
    {kWord,         "Word"},
    {kSubjectGroup, "Subject group"},
    {kBook,         "Book"},
    {kArticle,      "Article"},
    {kMovie,        "Movie"},
    {kSeries,       "Series"},
    {kAlbum,        "Album"},
    {kSong,         "Song"}
  };
}

QStringList SectionTypeManager::librarySectionNames() const {
  using enum LibrarySectionType;

  QStringList res;
  for (const auto e : enumRange(kWord, kSong)) {
    res << librarySectionTypeName(e);
  }

  return res;
}

QString SectionTypeManager::librarySectionTypeName(LibrarySectionType type) const {
  if (_typeNames.find(type) == _typeNames.end()) {
    return QString("Unknown section: %1").arg(static_cast<int>(type));
  }

  return _typeNames[type];
}

LibrarySectionType SectionTypeManager::librarySectionType(const QString& name) const {
  return _typeNames.key(name, LibrarySectionType::kUnknown);
}

LibrarySectionType SectionTypeManager::librarySectionType(int type) const {
  using enum LibrarySectionType;
  int bottom = std::to_underlying(kWord);
  int top = std::to_underlying(kSong);
  if (type < bottom || type > top) {
    return kUnknown;
  }
  return static_cast<LibrarySectionType>(type);
}

}
