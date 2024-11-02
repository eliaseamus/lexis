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

void LibrarySection::addItem(LibraryItem&& item) {
  _model->addItem(std::move(item));
}

void LibrarySection::updateItem(LibraryItem&& item) {
  _model->updateItem(std::move(item));
}

void LibrarySection::updateAudio(int id, QByteArray&& audio) {
  _model->updateAudio(id, std::move(audio));
}

void LibrarySection::removeItem(int id) {
  _model->removeItem(id);
}

}

