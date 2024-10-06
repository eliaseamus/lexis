#include "library_item_proxy_model.hpp"

namespace lexis {

LibraryItemProxyModel::LibraryItemProxyModel(QObject* parent) :
  QSortFilterProxyModel(parent),
  _source(new LibraryItemModel(this))
{
  setSourceModel(_source);
  setSortRole(LibraryItemModel::ModificationTimeRole);
  setFilterCaseSensitivity(Qt::CaseInsensitive);
}

void LibraryItemProxyModel::addItem(LibraryItem* item) {
  _source->addItem(item);
  sort(0, _sortOrder);
}

void LibraryItemProxyModel::toggleSort() {
  _sortOrder = _sortOrder == Qt::AscendingOrder ? Qt::DescendingOrder : Qt::AscendingOrder;
  sort(0, _sortOrder);
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
  QString author = index.data(LibraryItemModel::AuthorRole).toString();
  auto filter = filterRegularExpression();
  return title.contains(filter) || author.contains(filter);
}

}