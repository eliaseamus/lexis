#include "app_settings.hpp"

#include <QCoreApplication>

namespace lexis {

QHash<QString, LibraryItemModel::LibraryItemRole> AppSettings::sectionSortRoles() {
  static QHash<QString, LibraryItemModel::LibraryItemRole> roles = {
    {"Modification time", LibraryItemModel::ModificationTimeRole},
    {"Creation time", LibraryItemModel::CreationTimeRole},
    {"Title", LibraryItemModel::TitleRole}
  };
  return roles;
}

QList<QString> AppSettings::sectionSortRoleKeys() {
  const auto roles = sectionSortRoles();
  auto keys = roles.keys();
  keys.sort();
  return keys;
}

QString AppSettings::getSortRole() const {
  return _settings.value("sortRole").toString();
}

QString AppSettings::getCurrentLanguage() const {
  return _settings.value("currentLanguage").toString();
}

QString AppSettings::getInterfaceLanguage() const {
  return _settings.value("interfaceLanguage").toString();
}

}
