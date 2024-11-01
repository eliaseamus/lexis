#pragma once

#include <qqml.h>
#include <QSortFilterProxyModel>

#include "library_item_model.hpp"

namespace lexis {

class LibraryItemProxyModel : public QSortFilterProxyModel {
 Q_OBJECT
 QML_ELEMENT

 Q_PROPERTY(Qt::SortOrder sortOrder READ sortOrder NOTIFY dummy)

 private:
  LibraryItemModel* _source = nullptr;
  Qt::SortOrder _sortOrder = Qt::AscendingOrder;

 public:
  explicit LibraryItemProxyModel(QObject* parent = nullptr);
  void addItem(LibraryItem&& item);
  void updateItem(LibraryItem&& item);
  void removeItem(int id);
  bool isEmpty() const {return _source->isEmpty();}
  Qt::SortOrder sortOrder() const {return _sortOrder;}
  void reSort(const QString& role);
  Q_INVOKABLE void setSortingRole(const QString& role);
  Q_INVOKABLE void toggleSort();
  Q_INVOKABLE int size() const {return _source->rowCount();}

 protected:
  bool lessThan(const QModelIndex& lhs, const QModelIndex& rhs) const override;
  bool filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const override;

 signals:
  void dummy();
  void changed();
};

}
