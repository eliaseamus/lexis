#pragma once

#include <QWidget>
#include <QLineEdit>

#include "completer.hpp"

namespace lexis {

class StartPage : public QWidget {
 private:
  QLineEdit* _searchLine = nullptr;
  DictionaryCompleter* _completer = nullptr;

 public:
  StartPage(QWidget* parent = nullptr);
 
 private:
  void resizePage();
};

}

