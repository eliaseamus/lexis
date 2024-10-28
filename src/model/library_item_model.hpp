#pragma once

#include <qqml.h>

#include <QAbstractListModel>
#include <QStringList>

#include "library_item.hpp"

namespace lexis {

class LibraryItemModel : public QAbstractListModel {
 Q_OBJECT
 QML_ELEMENT

 private:
  QVector<LibraryItem*> _items;

 public:
  enum LibraryItemRole {
    IDRole = Qt::UserRole + 1,
    TitleRole,
    CreationTimeRole,
    ModificationTimeRole,
    TypeRole,
    ImageUrlRole,
    ColorRole
  };

  explicit LibraryItemModel(QObject* parent = nullptr);
  void addItem(LibraryItem&& item, QByteArray&& image);
  void updateItem(LibraryItem&& item, QByteArray&& image);
  void removeItem(int id);
  bool isEmpty() const {return _items.isEmpty();}
  int rowCount(const QModelIndex& parent = {}) const override;
  QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
  QHash<int, QByteArray> roleNames() const override;
};


}
