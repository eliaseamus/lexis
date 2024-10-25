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

void LibraryItemModel::updateItem(const QString& title, LibraryItem* item) {
  auto target = std::find_if(_items.begin(), _items.end(), [&title](auto* e){
    return e->title() == title;
  });
  if (target == _items.end()) {
    qWarning() << QString("Failed to update %1: no item with such title").arg(title);
    return;
  }
  item->setCreationTime((*target)->creationTime());
  emit layoutAboutToBeChanged();
  (*target)->init(item);
  emit layoutChanged();
}

void LibraryItemModel::removeItem(const QString& title) {
  auto item = std::find_if(_items.begin(), _items.end(), [&title](auto* item){
    return item->title() == title;
  });
  if (item == _items.end()) {
    qWarning() << QString("Failed to delete %1: no item with such title").arg(title);
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
