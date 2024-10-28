#include "library_item_model.hpp"

namespace lexis {

LibraryItemModel::LibraryItemModel(QObject* parent) :
  QAbstractListModel(parent)
{
}

void LibraryItemModel::addItem(LibraryItem&& item, QByteArray&& image) {
  beginInsertRows(QModelIndex(), rowCount(), rowCount());
  auto* newItem = new LibraryItem(this);
  newItem->init(std::move(item), std::move(image));
  _items.append(newItem);
  endInsertRows();
}

void LibraryItemModel::updateItem(LibraryItem&& item, QByteArray&& image) {
  auto target = std::find_if(_items.begin(), _items.end(), [&item](auto* e) {
    return e->id() == item.id();
  });
  if (target == _items.end()) {
    qWarning() << QString("Failed to update %1: no item with such id %2")
                         .arg(item.title(), item.id());
    return;
  }
  item.setCreationTime((*target)->creationTime());
  emit layoutAboutToBeChanged();
  (*target)->init(std::move(item), std::move(image));
  emit layoutChanged();
}

void LibraryItemModel::removeItem(int id) {
  auto item = std::find_if(_items.begin(), _items.end(), [id](auto* item){
    return item->id() == id;
  });
  if (item == _items.end()) {
    qWarning() << QString("Failed to delete %1: no item with such id").arg(id);
    return;
  }
  auto index = std::distance(_items.begin(), item);
  beginRemoveRows(QModelIndex(), index, index);
  _items.removeAt(index);
  endRemoveRows();
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
    case IDRole:
      return item->id();
    case TitleRole:
      return item->title();
    case TypeRole:
      return typeManager.librarySectionTypeName(item->type());
    case CreationTimeRole:
      return item->creationTime();
    case ModificationTimeRole:
      return item->modificationTime();
    case ImageUrlRole:
      return item->imageUrl();
    case ColorRole:
      return item->color();
  }

  return {};
}

QHash<int, QByteArray> LibraryItemModel::roleNames() const {
  QHash<int, QByteArray> roles;
  roles[IDRole] = "itemID";
  roles[TitleRole] = "title";
  roles[CreationTimeRole] = "creationTime";
  roles[ModificationTimeRole] = "modificationTime";
  roles[TypeRole] = "type";
  roles[ImageUrlRole] = "imageUrl";
  roles[ColorRole] = "itemColor";
  return roles;
}

}
