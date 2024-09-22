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
  createModel();
}

void Library::addItem(const LibraryItem* item) {
  auto row = _model->rowCount();
  if (!_model->insertRow(row)) {
    qDebug() << "failed to insert a new row";
  }

  _model->setData(_model->index(row, 1), item->title());
  _model->setData(_model->index(row, 2), item->type());

  if (!item->author().isEmpty()) {
    _model->setData(_model->index(row, 3), item->author());
  }

  if (item->year() != -1) {
    _model->setData(_model->index(row, 4), item->year());
    _model->setData(_model->index(row, 5), item->bc() ? 1 : 0);
  }

  if (!item->imageUrl().isEmpty()) {
    _model->setData(_model->index(row, 6), readFile(item->imageUrl()));
  }

  _model->setData(_model->index(row, 7), item->color().name());

  if (!_model->submitAll()) {
      qDebug() << "failed to submit library item" << _model->lastError();
  }
}

void Library::createTable() {
  QSqlQuery query;
  auto createTableQuery = "CREATE TABLE IF NOT EXISTS library        \
                          (id     INTEGER PRIMARY KEY AUTOINCREMENT, \
                           title  TEXT,                              \
                           type   TEXT,                              \
                           author TEXT,                              \
                           year   INTEGER,                           \
                           bc     INTEGER,                           \
                           image  BLOB,                              \
                           color  TEXT)";
  if (!query.exec(createTableQuery)) {
    qDebug() << "failed to create library table:" << query.lastError();
  }
}

void Library::createModel() {
  _model = new QSqlTableModel(this, _db);
  _model->setTable("library");
  _model->setEditStrategy(QSqlTableModel::OnManualSubmit);
  if (!_model->select()) {
    qDebug() << "Failed to populate library model";
  }
  int i = 0;
  if (!_model->setHeaderData(i++, Qt::Horizontal, "id")) {
    qDebug() << "failed to set header data: id" << _model->lastError();
  }
  _model->setHeaderData(i++, Qt::Horizontal, "title");
  _model->setHeaderData(i++, Qt::Horizontal, "type");
  _model->setHeaderData(i++, Qt::Horizontal, "author");
  _model->setHeaderData(i++, Qt::Horizontal, "year");
  _model->setHeaderData(i++, Qt::Horizontal, "bc");
  _model->setHeaderData(i++, Qt::Horizontal, "image");
  _model->setHeaderData(i++, Qt::Horizontal, "color");
}

QByteArray Library::readFile(const QUrl& url) {
  if (url.isEmpty()) {
    qDebug() << "url is empty";
    return {};
  }

  auto path = url.toLocalFile();
  QFile image(path);

  if (!image.open(QIODevice::ReadOnly)) {
    throw std::runtime_error(QString("Failed to open image: %1").arg(path).toStdString());
  }

  return image.readAll();
}

}
