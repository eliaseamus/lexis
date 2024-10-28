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
  static QHash<LibrarySectionType, QString> librarySectionTypeNames();

 public:
  explicit SectionTypeManager(QObject* parent = nullptr) : QObject(parent) {}

  Q_INVOKABLE QStringList librarySectionNames();
  Q_INVOKABLE QString librarySectionTypeName(LibrarySectionType type);
  Q_INVOKABLE LibrarySectionType librarySectionType(const QString& name);
  Q_INVOKABLE LibrarySectionType librarySectionType(int type);

};

}
