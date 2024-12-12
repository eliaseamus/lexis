#include "library_item.hpp"

#include <QFile>

#include "utils.hpp"

namespace lexis {

LibraryItem::LibraryItem(QObject* parent) : QObject(parent) {
  const auto fileTemplate = temporaryFileTemplate();
  _image.setFileTemplate(fileTemplate);
  _audio.setFileTemplate(fileTemplate);
}

LibraryItem::LibraryItem(LibraryItem&& item) {
  const auto fileTemplate = temporaryFileTemplate();
  _image.setFileTemplate(fileTemplate);
  _audio.setFileTemplate(fileTemplate);
  init(std::move(item));
}

LibraryItem& LibraryItem::operator=(LibraryItem&& item) {
  init(std::move(item));
  return *this;
}

void LibraryItem::freeAssets() {
  _image.setAutoRemove(true);
  _audio.setAutoRemove(true);
}

void LibraryItem::init(LibraryItem&& item) {
  _itemID = item._itemID;
  _title = std::move(item._title);
  _creationTime = std::move(item._creationTime);
  _modificationTime = std::move(item._modificationTime);
  _type = std::move(item._type);
  _color = std::move(item._color);
  _imageUrl = std::move(item._imageUrl);
  _image.setFileName(item._image.fileName());
  item._image.setAutoRemove(false);
  _audioUrl = std::move(item._audioUrl);
  _audio.setFileName(item._audio.fileName());
  item._audio.setAutoRemove(false);
  _meaning = std::move(item._meaning);
}

QByteArray LibraryItem::image() const {
  if (_imageUrl.isEmpty()) {
    return {};
  }
  return readFile(_imageUrl.toLocalFile());
}

QByteArray LibraryItem::audio() const {
  if (_audioUrl.isEmpty()) {
    return {};
  }
  return readFile(_audioUrl.toLocalFile());
}

void LibraryItem::setTitle(const QString& title) {
  _title = title;
  if (!_title.isEmpty()) {
    _title[0] = _title[0].toUpper();
  }
}

void LibraryItem::setType(const QString& typeName) {
  static SectionTypeManager typeManager;
  _type = typeManager.librarySectionType(typeName);
}

void LibraryItem::setImage(QByteArray&& data) {
  if (data.isEmpty()) {
    return;
  }
  writeFile(_image, std::move(data));
  _imageUrl = QUrl::fromLocalFile(_image.fileName());
}

void LibraryItem::setAudio(QByteArray&& data) {
  if (data.isEmpty()) {
    return;
  }
  writeFile(_audio, std::move(data));
  _audioUrl = QUrl::fromLocalFile(_audio.fileName());
}

QByteArray LibraryItem::readFile(const QString& path) const {
  QFile file(path);

  if (!file.open(QIODevice::ReadOnly)) {
    qWarning() << "fail to open file:" << path;
    return {};
  }

  return qCompress(file.readAll());
}

void LibraryItem::writeFile(QTemporaryFile& file, QByteArray&& data) {
  if (!file.open()) {
    qWarning() << "fail to open temporary file" << file.fileName();
    return;
  }
  file.resize(0);
  if (-1 == file.write(qUncompress(data))) {
    qWarning() << "Failed to overwrite file:" << file.errorString();
  }
  file.close();
}

}  // namespace lexis
