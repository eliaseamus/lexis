#pragma once

#include <QAbstractItemView>

namespace lexis {

class LibraryView : public QAbstractItemView {
 Q_OBJECT

 public:
  explicit LibraryView(QWidget* parent = nullptr);
};

}
