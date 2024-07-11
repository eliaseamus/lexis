#include "predictor.hpp"
#include "utils.hpp"

#include <QDebug>

namespace lexis {

Predictor::Predictor(QObject* parent) :
  QObject(parent)
{
  _manager = new QNetworkAccessManager(this);

  connect(_manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(replyFinished(QNetworkReply*)));

  qDebug() << MAKE_STR(PREDICTOR_API_KEY);
}


std::vector<std::string> Predictor::getPrediction(std::string_view query) {
  std::vector<std::string> res;

  return res;
}

void Predictor::replyFinished(QNetworkReply* reply) {

}

}


