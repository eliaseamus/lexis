#include "library_item_model.hpp"

namespace lexis {

LibraryItemModel::LibraryItemModel(QObject* parent) :
  QAbstractListModel(parent)
{
}

void LibraryItemModel::addItem(LibraryItem* item) {
  beginInsertRows(QModelIndex(), rowCount(), rowCount());
  _items.append(item);
  endInsertRows();
}

int LibraryItemModel::rowCount(const QModelIndex& parent) const {
  Q_UNUSED(parent);
  return _items.count();
}

QVariant LibraryItemModel::data(const QModelIndex& index, int role) const {
  if (index.row() < 0 || index.row() >= _items.count()) {
    return {};
  }

  const auto* item = _items[index.row()];
  SectionTypeManager typeManager;
  switch (role) {
    case TitleRole:
      return item->title();
    case TypeRole:
      return typeManager.librarySectionTypeName(item->type());
    case CreationTimeRole:
      return item->creationTime();
    case ModificationTimeRole:
      return item->modificationTime();
    case AuthorRole:
      return item->author();
    case YearRole:
      return item->year();
    case BcRole:
      return item->bc();
    case ImageUrlRole:
      return item->imageUrl();
    case ColorRole:
      return item->color();
  }

  return {};
}

QHash<int, QByteArray> LibraryItemModel::roleNames() const {
  QHash<int, QByteArray> roles;
  roles[TitleRole] = "title";
  roles[CreationTimeRole] = "creationTime";
  roles[ModificationTimeRole] = "modificationTime";
  roles[TypeRole] = "type";
  roles[AuthorRole] = "author";
  roles[YearRole] = "year";
  roles[BcRole] = "bc";
  roles[ImageUrlRole] = "imageUrl";
  roles[ColorRole] = "itemColor";
  return roles;
}

}
