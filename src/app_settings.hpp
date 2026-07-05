#pragma once

#include <qqml.h>

#include <QObject>
#include <QSettings>

#include "library_item_model.hpp"

#include "section_type.hpp"

namespace lexis {

class AppSettings : public QObject {
  Q_OBJECT
  QML_ELEMENT

 private:
  QSettings _settings;

 public:
  explicit AppSettings(QObject* parent = nullptr) : QObject(parent) {}
  Q_INVOKABLE QHash<QString, LibraryItemModel::LibraryItemRole> sectionSortRoles();
  Q_INVOKABLE QList<QString> sectionSortRoleKeys();
  Q_INVOKABLE QList<QString> wordSectionSortRoleKeys();
  Q_INVOKABLE QList<QString> groupSectionSortRoleKeys();
  QString getSortRole() const;
  QString getWordSortRole() const;
  QString getGroupSortRole() const;
  QString sortRoleForSectionType(LibrarySectionType type) const;
  Q_INVOKABLE void setWordSortRole(const QString& role);
  Q_INVOKABLE void setGroupSortRole(const QString& role);
  QString getCurrentLanguage() const;
  QString getInterfaceLanguage() const;
  QString getCurrentInterfaceLanguagePair() const;
};

}  // namespace lexis
