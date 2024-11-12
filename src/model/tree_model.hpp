#pragma once

#include <qqml.h>
#include <QAbstractItemModel>

#include <tree_item.hpp>

namespace lexis {

class TreeModel : public QAbstractItemModel {
 Q_OBJECT
 QML_ELEMENT

 private:
  std::unique_ptr<TreeItem> _root;

 public:
  explicit TreeModel(QObject* parent = nullptr);
  void setRoot(std::unique_ptr<TreeItem>&& root);
  Q_INVOKABLE QVariant data(const QModelIndex &index, int role) const override;
  Q_INVOKABLE Qt::ItemFlags flags(const QModelIndex &index) const override;
  Q_INVOKABLE QModelIndex index(int row, int column, const QModelIndex& parent = {}) const override;
  Q_INVOKABLE QModelIndex parent(const QModelIndex& index) const override;
  Q_INVOKABLE int rowCount(const QModelIndex& parent = {}) const override;
  Q_INVOKABLE int columnCount(const QModelIndex& parent = {}) const override;
};

}
