#pragma once

#include <QAbstractItemModel>

#include <tree_item.hpp>

namespace lexis {

class TreeModel : public QAbstractItemModel {
 Q_OBJECT

 private:
  std::unique_ptr<TreeItem> _root;

 public:
  explicit TreeModel(QObject* parent = nullptr);
  void setRoot(std::unique_ptr<TreeItem>&& root);
  QVariant data(const QModelIndex &index, int role) const override;
  Qt::ItemFlags flags(const QModelIndex &index) const override;
  QModelIndex index(int row, int column, const QModelIndex& parent = {}) const override;
  QModelIndex parent(const QModelIndex& index) const override;
  int rowCount(const QModelIndex& parent = {}) const override;
  int columnCount(const QModelIndex& parent = {}) const override;
};

}
