#include "tree_model.hpp"

namespace lexis {

namespace {

int itemId(const TreeModel& model, const QModelIndex& index) {
  return model.columnData(index, 1).toInt();
}

QModelIndex findIndexByIdRecursive(const TreeModel& model, const QModelIndex& parent, int targetId) {
  const int rows = model.rowCount(parent);
  for (int row = 0; row < rows; ++row) {
    const QModelIndex index = model.index(row, 0, parent);
    if (itemId(model, index) == targetId) {
      return index;
    }
    const auto found = findIndexByIdRecursive(model, index, targetId);
    if (found.isValid()) {
      return found;
    }
  }
  return {};
}

void collectMatches(const TreeModel& model, const QModelIndex& parent, const QString& query,
                    const QStringList& pathParts, QVariantList* results) {
  const int rows = model.rowCount(parent);
  for (int row = 0; row < rows; ++row) {
    const QModelIndex index = model.index(row, 0, parent);
    const auto title = model.data(index, Qt::DisplayRole).toString();
    auto path = pathParts;
    path.append(title);
    if (title.contains(query, Qt::CaseInsensitive)) {
      results->append(QVariantMap{
        {QStringLiteral("id"),         itemId(model, index)},
        {QStringLiteral("title"),      title                 },
        {QStringLiteral("breadcrumb"), path.join(QStringLiteral(" \u203a "))}
      });
    }
    collectMatches(model, index, query, path, results);
  }
}

}  // namespace

TreeModel::TreeModel(QObject* parent) : QAbstractItemModel(parent) {}

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

QVariant TreeModel::columnData(const QModelIndex& index, int column) const {
  if (!index.isValid()) {
    return {};
  }

  const auto* item = static_cast<const TreeItem*>(index.internalPointer());
  return column < item->columnCount() ? item->data(column) : QVariant{};
}

QModelIndex TreeModel::findIndexById(int itemId) const {
  return findIndexByIdRecursive(*this, {}, itemId);
}

QVariantList TreeModel::findMatches(const QString& query) const {
  QVariantList results;
  const auto trimmed = query.trimmed();
  if (trimmed.isEmpty()) {
    return results;
  }
  collectMatches(*this, {}, trimmed, {}, &results);
  return results;
}

Qt::ItemFlags TreeModel::flags(const QModelIndex& index) const {
  return index.isValid() ? QAbstractItemModel::flags(index) : Qt::ItemFlags(Qt::NoItemFlags);
}

QModelIndex TreeModel::index(int row, int column, const QModelIndex& parent) const {
  if (!hasIndex(row, column, parent)) {
    return {};
  }

  TreeItem* parentItem =
    parent.isValid() ? static_cast<TreeItem*>(parent.internalPointer()) : _root.get();

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

  return parentItem != _root.get() ? createIndex(parentItem->row(), 0, parentItem) : QModelIndex{};
}

int TreeModel::rowCount(const QModelIndex& parent) const {
  if (parent.column() > 0) {
    return 0;
  }

  const TreeItem* parentItem =
    parent.isValid() ? static_cast<const TreeItem*>(parent.internalPointer()) : _root.get();

  return parentItem->childCount();
}

int TreeModel::columnCount(const QModelIndex& parent) const {
  if (parent.isValid()) {
    return static_cast<TreeItem*>(parent.internalPointer())->columnCount();
  }
  return _root->columnCount();
}

}  // namespace lexis
