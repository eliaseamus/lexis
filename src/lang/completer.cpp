#include "completer.hpp"

namespace lexis {

Completer::Completer(QObject* parent) :
  QObject(parent)
{
  _predictor = new Predictor(this);
  _model = new QStringListModel(this);
  _completer = new QCompleter(_model, this);
  _completer->setCaseSensitivity(Qt::CaseInsensitive);
  _completer->setCompletionMode(QCompleter::UnfilteredPopupCompletion);
  _completer->setModelSorting(QCompleter::CaseInsensitivelySortedModel);

  connect(_predictor, SIGNAL(predictionsReceived(const QStringList&)), 
          this, SLOT(onPredictionsReceived(const QStringList&)));
}

void Completer::onTextEdited(const QString& text) {
  _predictor->request(text);
}

void Completer::onPredictionsReceived(const QStringList& predictions) {
  _model->setStringList(predictions);
}

}

