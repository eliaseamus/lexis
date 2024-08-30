#include "library.hpp"

namespace lexis {

Library::Library(QObject* parent) :
  QObject(parent)
{
  static const auto dbName = "lexis_library.db";
  _db = QSqlDatabase::addDatabase("QSQLITE");
  _db.setDatabaseName(dbName);
  if (!_db.open()) {
    throw std::runtime_error(QString("Failed to open %1").arg(dbName).toStdString());
  }
  createTable();
  _model = new QSqlTableModel(this, _db);
  _model->setTable("library");
  _model->setEditStrategy(QSqlTableModel::OnManualSubmit);
  if (_model->select()) {
    // throw std::runtime_error("Failed to populate library model");
  }
  if (!_model->setHeaderData(0, Qt::Horizontal, "id")) {
      qDebug() << "failed to set header data: id" << _model->lastError();
  }
  _model->setHeaderData(1, Qt::Horizontal, "title");
  _model->setHeaderData(2, Qt::Horizontal, "type");
  _model->setHeaderData(3, Qt::Horizontal, "image");
}

void Library::addItem(const LibraryItem& item) {
  auto row = _model->rowCount();
  if (!_model->insertRow(row)) {
    qDebug() << "failed to insert a new row";
  }
  if (!_model->setData(_model->index(row, 1), item.title)) {
    qDebug() << "failed to set data" << _model->lastError();
  }
  _model->setData(_model->index(row, 2), item.type);
  _model->setData(_model->index(row, 3), readImage(item.imageUrl));
  if (!_model->submitAll()) {
      qDebug() << "failed to submit library item" << _model->lastError();
  }
}

void Library::createTable() {
  QSqlQuery query;
  auto createTableQuery = "CREATE TABLE IF NOT EXISTS library \
                          (id INTEGER PRIMARY KEY AUTOINCREMENT, \
                           title TEXT, type text, image BLOB)";
  if (!query.exec(createTableQuery)) {
    qDebug() << "failed to create library table:" << query.lastError();
  }
}

QByteArray Library::readImage(const QUrl& url) {
  auto path = url.toLocalFile();
  QFile image(path);

  if (!image.open(QIODevice::ReadOnly)) {
      throw std::runtime_error(QString("Failed to open image: %1").arg(path).toStdString());
  }

  return image.readAll();
}

}

