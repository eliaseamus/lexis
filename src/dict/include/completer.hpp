#pragma once

#include <QCompleter>
#include "predictor.hpp"

namespace lexis {

class DictionaryCompleter : public QObject {
 private:
  QCompleter* _completer = nullptr;
  Predictor* _predictor = nullptr;

 public:
  DictionaryCompleter(QObject* parent);
  QCompleter* get() const {return _completer;}

};

}

