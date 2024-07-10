#pragma once

#include <QCompleter>

namespace lexis {

class DictionaryCompleter : public QObject {
 private:
  QCompleter* _completer = nullptr;

 public:
  DictionaryCompleter(QObject* parent);
  QCompleter* get() const {return _completer;}

};

}

