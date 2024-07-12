#pragma once

#include <QWidget>
#include <QLineEdit>
#include <QPushButton>

#include "completer.hpp"
#include "visualiser.hpp"

namespace lexis {

class StartPage : public QWidget {
 Q_OBJECT

 private:
  QLineEdit* _searchLine = nullptr;
  QPushButton* _searchButton = nullptr;
  DictionaryCompleter* _completer = nullptr;
  Visualiser* _visualiser = nullptr;

 public:
  StartPage(QWidget* parent = nullptr);
 
 private:
  void resizePage();

 private slots:
  void doSearch();
};

}

