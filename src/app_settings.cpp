#include "app_settings.hpp"

#include <QCoreApplication>

namespace lexis {

QHash<QString, LibraryItemModel::LibraryItemRole> AppSettings::sectionSortRoles() {
  static QHash<QString, LibraryItemModel::LibraryItemRole> roles = {
    {tr("Modification time"), LibraryItemModel::ModificationTimeRole},
    {tr("Creation time"), LibraryItemModel::CreationTimeRole},
    {tr("Title"), LibraryItemModel::TitleRole}
  };
  return roles;
}

QList<QString> AppSettings::sectionSortRoleKeys() {
  const auto roles = sectionSortRoles();
  auto keys = roles.keys();
  keys.sort();
  return keys;
}

QString AppSettings::getSavedSortRole() const {
  return _settings.value("sortRole").toString();
}

}
