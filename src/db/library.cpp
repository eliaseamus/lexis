#include "library.hpp"

#include <QtSql/QtSql>

#include "app_settings.hpp"

namespace lexis {

Library::Library(QObject* parent) :
  QObject(parent)
{
  openDatabase("lexis.db");
  AppSettings settings;
  openTable(settings.getCurrentLanguage());
}

void Library::openDatabase(const QString& name) {
  QFileInfo info(name);
  QDir dir;
  dir.mkpath(info.dir().path());
  QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
  db.setDatabaseName(info.filePath());
  if (!db.open()) {
    qDebug() << QString("Failed to open %1").arg(info.filePath()).toStdString();
  }
}

void Library::addItem(LibraryItem* item) {
  if (!item) {
    qWarning() << "no item was provided";
    return;
  }

  QSqlQuery query;
  query.prepare(
    QString(
      "INSERT INTO %1"
      "(title, creation_time, modification_time, type, image, color)"
      "VALUES (:title, :creation, :modification, :type, :image, :color)"
    ).arg(_table)
  );

  auto now = QDateTime::currentDateTime();
  item->setCreationTime(now);
  item->setModificationTime(now);
  query.bindValue(":title", item->title());
  query.bindValue(":creation", item->creationTime().toString());
  query.bindValue(":modification", item->modificationTime().toString());
  query.bindValue(":type", std::to_underlying(item->type()));
  query.bindValue(":image", item->image());
  query.bindValue(":color", item->color().name());

  if (!query.exec()) {
    qWarning() << "Failed to insert item into database:" << query.lastError();
    return;
  }

  item->setID(getItemID(item->title()));
  insertItem(std::move(*item), item->image());
}

void Library::updateItem(LibraryItem* item, LibrarySectionType oldType) {
  if (!item) {
    qWarning() << "no item was provided";
    return;
  }

  QSqlQuery query;
  query.prepare(
    QString(
      "UPDATE %1 "
      "SET title = :title, modification_time = :modification, type = :type, "
          "image = :image, color = :color "
      "WHERE id = \"%2\"").arg(_table, QString::number(item->id()))
  );

  item->setModificationTime(QDateTime::currentDateTime());
  query.bindValue(":title", item->title());
  query.bindValue(":modification", item->modificationTime().toString());
  query.bindValue(":type", std::to_underlying(item->type()));
  query.bindValue(":image", item->image());
  query.bindValue(":color", item->color().name());

  if (!query.exec()) {
    qWarning() << QString("Failed to update '%1' item in '%2' database:")
                  .arg(item->title(), _table) << query.lastError();
    return;
  }

  if (oldType == item->type()) {
    auto* section = getSection(item->type());
    section->updateItem(std::move(*item), std::move(item->image()));
  } else {
    auto* section = getSection(oldType);
    section->removeItem(item->id());
    if (section->isEmpty()) {
      _sections.removeOne(section);
    }
    insertItem(std::move(*item), item->image());
  }

}

void Library::deleteItem(int id, LibrarySectionType type) {
  QSqlQuery query;
  query.prepare(QString("DELETE FROM %1 WHERE id = \"%2\"").arg(_table, QString::number(id)));
  if (!query.exec()) {
    qWarning() << QString("Failed to delete item of id = '%1' from '%2' database:")
                  .arg(QString::number(id), _table) << query.lastError();
    return;
  }
  if (type != LibrarySectionType::kWord) {
    dropTableRecursively(QString("%1_%2").arg(_table, QString::number(id)));
  }
  auto* section = getSection(type);
  section->removeItem(id);
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
     image             BLOB,                              \
     color             TEXT)"
  ).arg(_table);
  if (!query.exec(createTableQuery)) {
    qDebug() << QString("failed to create %1 table").arg(_table) << query.lastError();
  }
}

void Library::createChildTable(const QString& parentTable, int parentID) {
  QSqlQuery query;
  auto createTableQuery = QString(
    "CREATE TABLE IF NOT EXISTS %1                        \
    (id                INTEGER PRIMARY KEY AUTOINCREMENT, \
     parent_id         INTEGER DEFAULT %2,                \
     creation_time     TEXT,                              \
     modification_time TEXT,                              \
     title             TEXT,                              \
     type              INTEGER,                           \
     image             BLOB,                              \
     color             TEXT,                              \
     FOREIGN KEY (parent_id) REFERENCES %3(parent_id) ON DELETE CASCADE)"
  ).arg(_table, QString::number(parentID), parentTable);
  if (!query.exec(createTableQuery)) {
    qDebug() << QString("failed to create %1 table").arg(_table) << query.lastError();
  }
}

void Library::openTable(const QString& name) {
  if (name.isEmpty()) {
    return;
  }
  _table = name;
  clearSections();
  createTable();
  populateSections();
}

void Library::openChildTable(int parentID) {
  auto parentTable = _table;
  _table = QString("%1_%2").arg(_table, QString::number(parentID));
  clearSections();
  createChildTable(parentTable, parentID);
  populateSections();
}

void Library::dropTableRecursively(const QString& root) {
  for (const auto& table : getTablesList()) {
    if (table.startsWith(root)) {
      dropTable(table);
    }
  }
}

int Library::getItemID(const QString& title) const {
  QSqlQuery query(
    QString(
      "SELECT id FROM %1 WHERE title = \"%2\""
    ).arg(_table, title)
  );
  int id = -1;
  while (query.next()) {
    id = query.value("id").toInt();
  }
  return id;
}

LibrarySectionType Library::getItemType(int id) const {
  QSqlQuery query(QString("SELECT type FROM %1 WHERE id = %2").arg(_table, id));
  LibrarySectionType type = LibrarySectionType::kUnknown;
  while (query.next()) {
    type = _typeManager.librarySectionType(query.value("type").toInt());
  }
  return type;
}

void Library::clearSections() {
  for (auto* section : _sections) {
    section->deleteLater();
  }
  _sections.clear();
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
      "SELECT id, title, creation_time, modification_time, type, image, color FROM %1"
    ).arg(_table)
  );

  while (query.next()) {
    LibraryItem item;

    item.setID(query.value("id").toInt());
    item.setTitle(query.value("title").toString());
    item.setCreationTime(QDateTime::fromString(query.value("creation_time").toString()));
    item.setModificationTime(QDateTime::fromString(query.value("modification_time").toString()));
    item.setType(_typeManager.librarySectionType(query.value("type").toInt()));
    item.setColor(query.value("color").toString());

    insertItem(std::move(item), query.value("image").toByteArray());
  }

}

void Library::insertItem(LibraryItem&& item, QByteArray&& image) {
  auto* section = getSection(item.type());
  section->addItem(std::move(item), std::move(image));
}

QStringList Library::getTablesList() const {
  QSqlQuery query("SELECT name FROM sqlite_master WHERE type='table'");
  QStringList list;
  while (query.next()) {
    list.append(query.value("name").toString());
  }
  return list;
}

void Library::dropTable(const QString& name) {
  if (name.isEmpty()) {
    return;
  }
  QSqlQuery query;
  auto deleteTableQuery = QString("DROP TABLE IF EXISTS %1").arg(name);
  if (!query.exec(deleteTableQuery)) {
    qDebug() << "failed to delete table" << name << query.lastError();
    return;
  }
  if (name == _table) {
    _sections.clear();
    _table.clear();
  }
}

}
