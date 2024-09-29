#pragma once

#include <qqml.h>

#include <QObject>
#include <QStringList>

#include "library_item_model.hpp"

namespace lexis {

class LibrarySection : public QObject {
 Q_OBJECT
 QML_ELEMENT

 Q_PROPERTY(LibrarySectionType type READ type NOTIFY dummy);
 Q_PROPERTY(QString name READ name NOTIFY dummy);
 Q_PROPERTY(LibraryItemModel* model READ model NOTIFY dummy);

 private:
  LibrarySectionType _type;
  QString _name;
  LibraryItemModel* _model = nullptr;

 public:
  LibrarySection(LibrarySectionType type, QObject* parent = nullptr);

  LibrarySectionType type() const {return _type;}
  QString name() const {return _name;}
  LibraryItemModel* model() const {return _model;}

  void addItem(LibraryItem* item);

 signals:
  void dummy();
};

}
