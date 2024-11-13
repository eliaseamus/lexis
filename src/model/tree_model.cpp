#include "tree_model.hpp"

namespace lexis {

TreeModel::TreeModel(QObject* parent) :
  QAbstractItemModel(parent)
{}

void TreeModel::setRoot(std::unique_ptr<TreeItem>&& root) {
  _root = std::move(root);
}

QVariant TreeModel::data(const QModelIndex& index, int role) const {
  if (!index.isValid() || role != Qt::DisplayRole) {
    return {};
  }

  const auto* item = static_cast<const TreeItem*>(index.internalPointer());
  return item->data(index.column());
}

QVariant TreeModel::columnData(const QModelIndex& index, int column) {
  if (!index.isValid()) {
    return {};
  }

  const auto* item = static_cast<const TreeItem*>(index.internalPointer());
  return column < item->columnCount() ? item->data(column) : QVariant{};
}

Qt::ItemFlags TreeModel::flags(const QModelIndex& index) const {
  return index.isValid() ? QAbstractItemModel::flags(index) : Qt::ItemFlags(Qt::NoItemFlags);
}

QModelIndex TreeModel::index(int row, int column, const QModelIndex& parent) const {
  if (!hasIndex(row, column, parent)) {
    return {};
  }

  TreeItem* parentItem = parent.isValid() ?
                         static_cast<TreeItem*>(parent.internalPointer()) :
                         _root.get();

  if (auto* childItem = parentItem->child(row)) {
    return createIndex(row, column, childItem);
  }

  return {};
}

QModelIndex TreeModel::parent(const QModelIndex& index) const {
  if (!index.isValid()) {
    return {};
  }

  auto* childItem = static_cast<TreeItem*>(index.internalPointer());
  TreeItem* parentItem = childItem->parent();

  return parentItem != _root.get() ?
                       createIndex(parentItem->row(), 0, parentItem) :
                       QModelIndex{};
}

int TreeModel::rowCount(const QModelIndex& parent) const {
  if (parent.column() > 0) {
    return 0;
  }

  const TreeItem* parentItem = parent.isValid() ?
                               static_cast<const TreeItem*>(parent.internalPointer()) :
                               _root.get();

  return parentItem->childCount();
}

int TreeModel::columnCount(const QModelIndex& parent) const {
  if (parent.isValid()) {
    return static_cast<TreeItem*>(parent.internalPointer())->columnCount();
  }
  return _root->columnCount();
}

}
