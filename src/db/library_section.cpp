#include "library_section.hpp"

#include <QHash>

namespace lexis {

LibrarySection::LibrarySection(LibrarySectionType type, QObject* parent) :
  QObject(parent),
  _type(type),
  _name(SectionTypeManager::librarySectionTypeName(type)),
  _model(new LibraryItemModel(this))
{
}

void LibrarySection::addItem(LibraryItem* item) {
  _model->addItem(item);
}

}

