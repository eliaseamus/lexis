#include "library_item_proxy_model.hpp"

#include "app_settings.hpp"

namespace lexis {

LibraryItemProxyModel::LibraryItemProxyModel(QObject* parent) :
  QSortFilterProxyModel(parent),
  _source(new LibraryItemModel(this))
{
  AppSettings settings;
  setSourceModel(_source);
  setSortingRole(settings.getSortRole());
  setFilterCaseSensitivity(Qt::CaseInsensitive);
}

void LibraryItemProxyModel::addItem(LibraryItem&& item) {
  _source->addItem(std::move(item));
  sort(0, _sortOrder);
  emit changed();
}

void LibraryItemProxyModel::updateItem(LibraryItem&& item) {
  _source->updateItem(std::move(item));
  emit changed();
}

void LibraryItemProxyModel::updateAudio(int id, QByteArray&& audio) {
  _source->updateAudio(id, std::move(audio));
  emit changed();
}

void LibraryItemProxyModel::updateMeaning(int id, const QString& meaning) {
  _source->updateMeaning(id, meaning);
  emit changed();
}

void LibraryItemProxyModel::removeItem(int id) {
  _source->removeItem(id);
  emit changed();
}

void LibraryItemProxyModel::reSort(const QString& role) {
  setSortingRole(role);
  sort(0, _sortOrder);
  emit changed();
}

void LibraryItemProxyModel::setSortingRole(const QString& role) {
  static QHash<QString, LibraryItemModel::LibraryItemRole> roles = {
    {"Modification time", LibraryItemModel::ModificationTimeRole},
    {"Creation time", LibraryItemModel::CreationTimeRole},
    {"Title", LibraryItemModel::TitleRole}
  };

  if (!roles.contains(role)) {
    qWarning() << "Unknown sort role:" << role;
    setSortRole(LibraryItemModel::ModificationTimeRole);
    return;
  }

  setSortRole(roles[role]);
}

void LibraryItemProxyModel::toggleSort() {
  _sortOrder = _sortOrder == Qt::AscendingOrder ? Qt::DescendingOrder : Qt::AscendingOrder;
  sort(0, _sortOrder);
  emit changed();
}

bool LibraryItemProxyModel::lessThan(const QModelIndex& lhs, const QModelIndex& rhs) const {
  QVariant leftData = sourceModel()->data(lhs, sortRole());
  QVariant rightData = sourceModel()->data(rhs, sortRole());

  switch (leftData.userType()) {
    case QMetaType::QDateTime:
      return leftData.toDateTime() < rightData.toDateTime();
    case QMetaType::QString:
      return QString::localeAwareCompare(leftData.toString(), rightData.toString()) < 0;
  }

  qWarning() << "Unknown type for sorting";
  return false;
}

bool LibraryItemProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const {
  QModelIndex index = _source->index(sourceRow, 0, sourceParent);
  QString title = index.data(LibraryItemModel::TitleRole).toString();
  auto filter = filterRegularExpression();
  return title.contains(filter);
}

}
