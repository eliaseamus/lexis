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
  void addItem(LibraryItem* item);
  Qt::SortOrder sortOrder() const {return _sortOrder;}
  void reSort(const QString& role);
  Q_INVOKABLE void setSortingRole(const QString& role);
  Q_INVOKABLE void toggleSort();

 protected:
  bool lessThan(const QModelIndex& lhs, const QModelIndex& rhs) const override;
  bool filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const override;

 private:
  QString getSavedSortRole() const;

 signals:
  void dummy();
};

}
