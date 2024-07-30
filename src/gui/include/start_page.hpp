#pragma once

#include <QWidget>
#include <QLineEdit>
#include <QPushButton>

#include "completer.hpp"

namespace lexis {

class StartPage : public QWidget {
 Q_OBJECT

 private:
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

