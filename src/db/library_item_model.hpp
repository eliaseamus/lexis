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
  QList<LibraryItem*> _items;

 public:
  enum LibraryItemRoles {
    TitleRole = Qt::UserRole + 1,
    TypeRole,
    AuthorRole,
    YearRole,
    BcRole,
    ImageUrlRole,
    ColorRole
  };

  LibraryItemModel(QObject* parent = nullptr);
  void addItem(LibraryItem* item);
  int rowCount(const QModelIndex& parent = {}) const override;
  QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
  QHash<int, QByteArray> roleNames() const override;
};


}
