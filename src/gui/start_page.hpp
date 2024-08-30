#pragma once

#include <QWidget>
#include <QLineEdit>
#include <QPushButton>

#include "library.hpp"
#include "library_view.hpp"
#include "completer.hpp"

namespace lexis {

class StartPage : public QWidget {
 Q_OBJECT

 private:
  Library* _library = nullptr;
  // LibraryView* _view = nullptr;
  QLineEdit* _searchLine = nullptr;
  QPushButton* _searchButton = nullptr;
  Completer* _completer = nullptr;
  QPushButton* _addItem = nullptr;

 public:
  explicit StartPage(QWidget* parent = nullptr);
 
 private:
  void resizeWindow();

 private slots:
  void doSearch();
  void addItem();
};

}

