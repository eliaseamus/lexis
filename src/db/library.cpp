#include "library.hpp"

#include "app_settings.hpp"

#include <QtSql/QtSql>

namespace lexis {

Library::Library(QObject* parent) :
  QObject(parent)
{
  static const auto dbName = "lexis_library.db";
  QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
  db.setDatabaseName(dbName);
  if (!db.open()) {
    qDebug() << QString("Failed to open %1").arg(dbName).toStdString();
  }
  AppSettings settings;
  changeLanguage(settings.getCurrentLanguage());
}

void Library::addItem(LibraryItem* item) {
  QSqlQuery query;
  query.prepare(
    QString(
      "INSERT INTO %1"
      "(title, creation_time, modification_time, type, author, year, bc, image, color)"
      "VALUES (:title, :creation, :modification, :type, :author, :year, :bc, :image, :color)"
    ).arg(_language)
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

void Library::deleteItem(LibrarySectionType sectionType, const QString& title) {
  QSqlQuery query;
  query.prepare(QString("DELETE FROM %1 WHERE title = \"%2\"").arg(_language, title));
  if (!query.exec()) {
    qWarning() << QString("Failed to delete \"%1\" item from \"%2\" database:")
                  .arg(title, _language) << query.lastError();
    return;
  }
  auto* section = getSection(sectionType);
  section->removeItem(title);
  if (section->isEmpty()) {
    _sections.removeOne(section);
  }
}

void Library::createTable() {
  QSqlQuery query;
  auto createTableQuery = QString(
    "CREATE TABLE IF NOT EXISTS %1                        \
    (id                INTEGER PRIMARY KEY AUTOINCREMENT, \
     creation_time     TEXT,                              \
     modification_time TEXT,                              \
     title             TEXT,                              \
     type              INTEGER,                           \
     author            TEXT,                              \
     year              INTEGER,                           \
     bc                INTEGER,                           \
     image             BLOB,                              \
     color             TEXT)"
  ).arg(_language);
  if (!query.exec(createTableQuery)) {
    qDebug() << "failed to create language table" << _language << query.lastError();
  }
}

void Library::changeLanguage(const QString& language) {
  if (language.isEmpty()) {
    return;
  }
  _sections.clear();
  _language = language;
  createTable();
  populateSections();
}

void Library::deleteLanguage(const QString& language) {
  if (language.isEmpty()) {
    return;
  }
  QSqlQuery query;
  auto deleteTableQuery = QString("DROP TABLE %1").arg(language);
  if (!query.exec(deleteTableQuery)) {
    qDebug() << "failed to delete language table" << language << query.lastError();
  }
  if (language == _language) {
    _sections.clear();
    _language.clear();
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
    QString(
      "SELECT title, creation_time, modification_time, type,"
      "       author, year, bc, image, color FROM %1"
    ).arg(_language)
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
