#pragma once

#include <qqml.h>

#include <QObject>
#include <QString>

namespace lexis {
Q_NAMESPACE

enum class LibrarySectionType {
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

 public:
  explicit SectionTypeManager(QObject* parent = nullptr) : QObject(parent) {}

  Q_INVOKABLE static QStringList librarySectionNames();
  Q_INVOKABLE static QString librarySectionTypeName(LibrarySectionType type);
  Q_INVOKABLE static LibrarySectionType librarySectionType(const QString& name);
  static LibrarySectionType librarySectionType(int type);

};

}
