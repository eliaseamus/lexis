#include "library.hpp"
#include "utils.hpp"

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
}

void Library::addItem(LibraryItem* item) {
  QSqlQuery query;
  query.prepare("INSERT INTO library"
                "(title, type, author, year, bc, image, color)"
                "VALUES (:title, :type, :author, :year, :bc, :image, :color)");

  query.bindValue(":title", item->title());
  query.bindValue(":type", item->type());
  query.bindValue(":author", item->author());
  query.bindValue(":year", item->year());
  query.bindValue(":bc", item->bc() ? 1 : 0);
  query.bindValue(":image", readFile(item->imageUrl()));
  query.bindValue(":color", item->color().name());

  if (!query.exec()) {
    qWarning() << "Failed to insert item into database:" << query.lastError();
    return;
  }
}

QVector<QStringList> Library::getItemsOfType(const QString& type) const {
  return {};
}

void Library::createTable() {
  QSqlQuery query;
  auto createTableQuery = "CREATE TABLE IF NOT EXISTS library        \
                          (id     INTEGER PRIMARY KEY AUTOINCREMENT, \
                           title  TEXT,                              \
                           type   TEXT,                              \
                           author TEXT,                              \
                           year   TEXT,                              \
                           bc     INTEGER,                           \
                           image  BLOB,                              \
                           color  TEXT)";
  if (!query.exec(createTableQuery)) {
    qDebug() << "failed to create library table:" << query.lastError();
  }
}

}
