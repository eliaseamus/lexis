#pragma once

#include <qqml.h>
#include <QObject>
#include <QSettings>

#include "library_item_model.hpp"

namespace lexis {

class AppSettings : public QObject {
 Q_OBJECT
 QML_ELEMENT

 private:
  QSettings _settings;

 public:
   explicit AppSettings (QObject *parent = nullptr) : QObject (parent) {}
   Q_INVOKABLE QHash<QString, LibraryItemModel::LibraryItemRole> sectionSortRoles();
   Q_INVOKABLE QList<QString> sectionSortRoleKeys();
   QString getSortRole() const;
   QString getInputLanguage() const;
};

}
