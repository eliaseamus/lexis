#pragma once

#include <qqml.h>

#include <QObject>
#include <QStringList>

#include "library_item_proxy_model.hpp"

namespace lexis {

class LibrarySection : public QObject {
 Q_OBJECT
 QML_ELEMENT

 Q_PROPERTY(LibrarySectionType type READ type WRITE setType NOTIFY dummy);
 Q_PROPERTY(QString name READ name NOTIFY dummy);
 Q_PROPERTY(LibraryItemProxyModel* model READ model NOTIFY dummy);

 private:
  LibraryItemProxyModel* _model = nullptr;
  LibrarySectionType _type;
  QString _name;

 public:
  explicit LibrarySection(QObject* parent = nullptr);
  LibrarySection(LibrarySectionType type, QObject* parent = nullptr);

  LibrarySectionType type() const {return _type;}
  QString name() const {return _name;}
  LibraryItemProxyModel* model() const {return _model;}

  void setType(LibrarySectionType type);
  void addItem(LibraryItem* item);

 signals:
  void dummy();
};

}
