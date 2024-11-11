#pragma once

#include <QString>
#include <QVariant>
#include <memory>

namespace lexis {

class TreeItem {
 private:
  QVariantList _data;
  std::vector<std::unique_ptr<TreeItem>> _childItems;
  TreeItem* _parent = nullptr;

 public:
  explicit TreeItem(QVariantList data, TreeItem* parent = nullptr);
  void appendChild(std::unique_ptr<TreeItem>&& child);
  TreeItem* child(int row);
  int childCount() const;
  int columnCount() const;
  QVariant data(int column) const;
  int row() const;
  TreeItem* parent() {return _parent;}
};

}
