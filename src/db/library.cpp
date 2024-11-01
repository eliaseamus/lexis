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
    qWarning() << QString("Failed to insert item into '%1' table:").arg(_table)
               << query.lastError();
    return;
  }

  item->setID(getItemID(item->title()));
  updateParentModificationTime(_table, item->id());
  insertItem(std::move(*item));
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
    qWarning() << QString("Failed to update '%1' item in '%2' table:")
                  .arg(item->title(), _table) << query.lastError();
    return;
  }

  updateParentModificationTime(_table, item->id());

  if (oldType == item->type()) {
    auto* section = getSection(item->type());
    section->updateItem(std::move(*item));
  } else {
    auto* section = getSection(oldType);
    section->removeItem(item->id());
    if (section->isEmpty()) {
      _sections.removeOne(section);
    }
    insertItem(std::move(*item));
  }

}

void Library::deleteItem(int id, LibrarySectionType type) {
  updateParentModificationTime(_table, id);
  QSqlQuery query;
  query.prepare(QString("DELETE FROM %1 WHERE id = \"%2\"").arg(_table, QString::number(id)));
  if (!query.exec()) {
    qWarning() << QString("Failed to delete item of id = '%1' from '%2' table:")
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
     color             TEXT,                              \
     audio             BLOB)"
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
     parent_table      TEXT DEFAULT %2,                   \
     parent_id         INTEGER DEFAULT %3,                \
     creation_time     TEXT,                              \
     modification_time TEXT,                              \
     title             TEXT,                              \
     type              INTEGER,                           \
     image             BLOB,                              \
     color             TEXT,                              \
     audio             BLOB)"
  ).arg(_table, parentTable, QString::number(parentID));
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
      "SELECT id, title, creation_time, modification_time, type, image, color, audio FROM %1"
    ).arg(_table)
  );

  while (query.next()) {
    LibraryItem item;

    item.setID(query.value("id").toInt());
    item.setTitle(query.value("title").toString());
    item.setCreationTime(QDateTime::fromString(query.value("creation_time").toString()));
    item.setModificationTime(QDateTime::fromString(query.value("modification_time").toString()));
    item.setType(_typeManager.librarySectionType(query.value("type").toInt()));
    item.setImage(query.value("image").toByteArray());
    item.setColor(query.value("color").toString());
    item.setAudio(query.value("audio").toByteArray());

    insertItem(std::move(item));
  }

}

void Library::insertItem(LibraryItem&& item) {
  auto* section = getSection(item.type());
  section->addItem(std::move(item));
}

QStringList Library::getTablesList() const {
  QSqlQuery query("SELECT name FROM sqlite_master WHERE type = 'table'");
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

void Library::updateParentModificationTime(const QString& table, int id) {
  QSqlQuery query(
    QString(
      "SELECT parent_table, parent_id FROM %1 WHERE id = \"%2\""
    ).arg(table, QString::number(id))
  );
  while (query.next()) {
    auto parentTable = query.value("parent_table").toString();
    auto parentID = query.value("parent_id").toInt();
    QSqlQuery updateQuery;
    updateQuery.prepare(
      QString(
        "UPDATE %1 "
        "SET modification_time = :modification "
        "WHERE id = \"%2\"").arg(parentTable, QString::number(parentID))
    );
    updateQuery.bindValue(":modification", QDateTime::currentDateTime().toString());
    if (!updateQuery.exec()) {
      qWarning() << QString("Failed to update '%1' item in '%2' table:")
                    .arg(QString::number(parentID), parentTable) << query.lastError();
      return;
    }
    updateParentModificationTime(parentTable, parentID);
  }
}

}
