#pragma once

#include "predictor.hpp"
#include <QCompleter>
#include <QStringListModel>

namespace lexis {

class Completer : public QObject {
 Q_OBJECT

 private:
  QCompleter* _completer = nullptr;
  Predictor* _predictor = nullptr;
  QStringListModel* _model = nullptr;

 public:
  explicit Completer(QObject* parent);
  QCompleter* get() const {return _completer;}

 public slots:
  void onTextEdited(const QString& text);
  void onPredictionsReceived(const QStringList& predictions);

};

}

