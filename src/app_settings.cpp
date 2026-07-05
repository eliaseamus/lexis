#include "app_settings.hpp"

#include <QCoreApplication>

#include "section_type.hpp"

namespace lexis {

namespace {

QString legacySortRole(const QSettings& settings) {
  return settings.value(QStringLiteral("sortRole")).toString();
}

QString normalizedSortRole(const QString& role, bool allowFrequency) {
  static const QHash<QString, LibraryItemModel::LibraryItemRole> roles = {
    {"Modification time", LibraryItemModel::ModificationTimeRole},
    {"Creation time",     LibraryItemModel::CreationTimeRole    },
    {"Frequency",         LibraryItemModel::FrequencyRankRole   },
    {"Title",             LibraryItemModel::TitleRole           }
  };

  if (!allowFrequency && role == QStringLiteral("Frequency")) {
    return QStringLiteral("Title");
  }
  if (roles.contains(role)) {
    return role;
  }
  return QStringLiteral("Title");
}

}  // namespace

QHash<QString, LibraryItemModel::LibraryItemRole> AppSettings::sectionSortRoles() {
  static QHash<QString, LibraryItemModel::LibraryItemRole> roles = {
    {"Modification time", LibraryItemModel::ModificationTimeRole},
    {"Creation time",     LibraryItemModel::CreationTimeRole    },
    {"Frequency",         LibraryItemModel::FrequencyRankRole   },
    {"Title",             LibraryItemModel::TitleRole           }
  };
  return roles;
}

QList<QString> AppSettings::sectionSortRoleKeys() {
  const auto roles = sectionSortRoles();
  auto keys = roles.keys();
  keys.sort();
  return keys;
}

QList<QString> AppSettings::wordSectionSortRoleKeys() {
  return sectionSortRoleKeys();
}

QList<QString> AppSettings::groupSectionSortRoleKeys() {
  auto keys = sectionSortRoleKeys();
  keys.removeAll(QStringLiteral("Frequency"));
  return keys;
}

QString AppSettings::getSortRole() const {
  return getWordSortRole();
}

QString AppSettings::getWordSortRole() const {
  const auto role = _settings.value(QStringLiteral("wordSortRole")).toString();
  if (!role.isEmpty()) {
    return normalizedSortRole(role, true);
  }
  return normalizedSortRole(legacySortRole(_settings), true);
}

QString AppSettings::getGroupSortRole() const {
  const auto role = _settings.value(QStringLiteral("groupSortRole")).toString();
  if (!role.isEmpty()) {
    return normalizedSortRole(role, false);
  }
  return normalizedSortRole(legacySortRole(_settings), false);
}

QString AppSettings::sortRoleForSectionType(LibrarySectionType type) const {
  return type == LibrarySectionType::kWord ? getWordSortRole() : getGroupSortRole();
}

void AppSettings::setWordSortRole(const QString& role) {
  _settings.setValue(QStringLiteral("wordSortRole"), normalizedSortRole(role, true));
}

void AppSettings::setGroupSortRole(const QString& role) {
  _settings.setValue(QStringLiteral("groupSortRole"), normalizedSortRole(role, false));
}

QString AppSettings::getCurrentLanguage() const {
  return _settings.value("currentLanguage").toString();
}

QString AppSettings::getInterfaceLanguage() const {
  return _settings.value("interfaceLanguage").toString();
}

QString AppSettings::getCurrentInterfaceLanguagePair() const {
  return QString("%1-%2").arg(getCurrentLanguage(), getInterfaceLanguage());
}

}  // namespace lexis
