#include "library.hpp"

namespace lexis {

Library::Library(QObject* parent) :
  QObject(parent)
{
  static const auto dbName = "lexis_library.db";
  _db = QSqlDatabase::addDatabase("QSQLITE");
  _db.setDatabaseName(dbName);
  if (!_db.open()) {
    qDebug() << QString("Failed to open %1").arg(dbName).toStdString();
  }
  createTable();
  populateSections();
}

void Library::addItem(LibraryItem* item) {
  QSqlQuery query;
  query.prepare(
    "INSERT INTO library"
    "(title, creation_time, modification_time, type, author, year, bc, image, color)"
    "VALUES (:title, :creation, :modification, :type, :author, :year, :bc, :image, :color)"
  );

  query.bindValue(":title", item->title());
  query.bindValue(":creation", item->creationTime().toString());
  query.bindValue(":modification", item->modificationTime().toString());
  query.bindValue(":type", std::to_underlying(item->type()));
  query.bindValue(":author", item->author());
  query.bindValue(":year", item->year());
  query.bindValue(":bc", item->bc() ? 1 : 0);
  query.bindValue(":image", item->image());
  query.bindValue(":color", item->color().name());

  if (!query.exec()) {
    qWarning() << "Failed to insert item into database:" << query.lastError();
    return;
  }

  auto* newItem = new LibraryItem(this);
  newItem->init(item);
  updateSections(newItem);
}

void Library::createTable() {
  QSqlQuery query;
  auto createTableQuery = "CREATE TABLE IF NOT EXISTS library                   \
                          (id                INTEGER PRIMARY KEY AUTOINCREMENT, \
                           creation_time     TEXT,                              \
                           modification_time TEXT,                              \
                           title             TEXT,                              \
                           type              INTEGER,                           \
                           author            TEXT,                              \
                           year              INTEGER,                           \
                           bc                INTEGER,                           \
                           image             BLOB,                              \
                           color             TEXT)";
  if (!query.exec(createTableQuery)) {
    qDebug() << "failed to create library table:" << query.lastError();
  }
}

LibrarySection* Library::getSection(LibrarySectionType type) {
  auto pos = std::find_if(_sections.begin(), _sections.end(), [type](LibrarySection* section) {
    return section->type() == type;
  });

  if (pos != _sections.end()) {
    return *pos;
  }

  auto* section = new LibrarySection(type, this);
  _sections.push_back(section);
  return section;
}

void Library::populateSections() {
  QSqlQuery query(
    "SELECT title, creation_time, modification_time, type,"
    "       author, year, bc, image, color FROM library"
  );

  SectionTypeManager typeManager;
  while (query.next()) {
    auto* item = new LibraryItem(this);

    item->setTitle(query.value("title").toString());
    item->setCreationTime(QDateTime::fromString(query.value("creation_time").toString()));
    item->setModificationTime(QDateTime::fromString(query.value("modification_time").toString()));
    item->setType(typeManager.librarySectionType(query.value("type").toInt()));
    item->setAuthor(query.value("author").toString());
    item->setYear(query.value("year").toInt());
    item->setBc(static_cast<bool>(query.value("bc").toInt()));
    item->setImage(query.value("image").toByteArray());
    item->setColor(query.value("color").toString());

    updateSections(item);
  }

}

void Library::updateSections(LibraryItem* item) {
  auto* section = getSection(item->type());
  section->addItem(item);
}

}
