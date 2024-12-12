#pragma once

#include <qqml.h>

#include <QObject>
#include <QString>

namespace lexis {
Q_NAMESPACE

enum class LibrarySectionType {
  kWord,
  kSubjectGroup,
  kBook,
  kArticle,
  kMovie,
  kSeries,
  kAlbum,
  kSong,
  kUnknown
};

Q_ENUM_NS(LibrarySectionType);

class SectionTypeManager : public QObject {
  Q_OBJECT
  QML_ELEMENT

 private:
  QHash<LibrarySectionType, QString> _typeNames;

 public:
  explicit SectionTypeManager(QObject* parent = nullptr);
  Q_INVOKABLE QStringList librarySectionNames() const;
  Q_INVOKABLE QString librarySectionTypeName(LibrarySectionType type) const;
  Q_INVOKABLE LibrarySectionType librarySectionType(const QString& name) const;
  Q_INVOKABLE LibrarySectionType librarySectionType(int type) const;
};

}  // namespace lexis
