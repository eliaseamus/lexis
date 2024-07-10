#include "completer.hpp"

namespace lexis {

DictionaryCompleter::DictionaryCompleter(QObject* parent) :
  QObject(parent)
{
  QStringList wordList;
  wordList << "alpha" << "omega" << "omicron" << "zeta";
  _completer = new QCompleter(wordList, this); 
}

}

