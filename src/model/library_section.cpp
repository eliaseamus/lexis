#include "library_section.hpp"

#include <QHash>

namespace lexis {

LibrarySection::LibrarySection(QObject* parent) :
  QObject(parent),
  _model(new LibraryItemProxyModel(this))
{
}

LibrarySection::LibrarySection(LibrarySectionType type, QObject* parent) :
  LibrarySection(parent)
{
  setType(type);
}

void LibrarySection::sort(const QString& sortRole) {
  _model->reSort(sortRole);
}

void LibrarySection::setType(LibrarySectionType type) {
  _type = type;
  _name = SectionTypeManager::librarySectionTypeName(type);
}

void LibrarySection::addItem(LibraryItem* item) {
  _model->addItem(item);
}

}

