#include "tree_item.hpp"

namespace lexis {

TreeItem::TreeItem(QVariantList data, TreeItem* parent) :
  _data(std::move(data)),
  _parent(parent)
{}

void TreeItem::appendChild(std::unique_ptr<TreeItem>&& child) {
  _childItems.emplace_back(std::move(child));
}

TreeItem* TreeItem::child(int row) {
  TreeItem* res = nullptr;
  if (row >= 0 && row < childCount()) {
    res = _childItems.at(row).get();
  }
  return res;
}

int TreeItem::childCount() const {
  return int(_childItems.size());
}

int TreeItem::columnCount() const {
  return int(_data.count());
}

QVariant TreeItem::data(int column) const {
  return _data.value(column);
}

int TreeItem::row() const {
  if (_parent == nullptr) {
    return 0;
  }

  auto searchFunc = [this](const std::unique_ptr<TreeItem>& treeItem) {
    return treeItem.get() == this;
  };
  const auto it = std::find_if(_parent->_childItems.cbegin(),
                               _parent->_childItems.cend(),
                               searchFunc);

  if (it != _parent->_childItems.cend()) {
    return std::distance(_parent->_childItems.cbegin(), it);
  }

  Q_ASSERT(false);

  return -1;
}

}
