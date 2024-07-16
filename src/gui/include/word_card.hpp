#pragma once

#include <QWidget>
#include <QLabel>

#include "dictionary.hpp"

namespace lexis {

class WordCard : public QWidget {
 Q_OBJECT

 private:
  Dictionary* _dict = nullptr;
  QLabel* _title = nullptr;

 public:
  WordCard(QWidget* parent = nullptr);
  WordCard(const QString& word, QWidget* parent = nullptr);
  void build(const QString& word);

 public slots:
  void displayDefinitions(const QVector<Definition>& definitions);
};

}

