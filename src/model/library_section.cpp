#include "library_section.hpp"

#include <QHash>

#include "section_type.hpp"

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
  static SectionTypeManager typeManager;
  _type = type;
  _name = typeManager.librarySectionTypeName(type);
}

void LibrarySection::addItem(LibraryItem* item) {
  _model->addItem(item);
}

void LibrarySection::updateItem(const QString& title, LibraryItem* item) {
  _model->updateItem(title, item);
}

void LibrarySection::removeItem(const QString& title) {
  _model->removeItem(title);
}

}

