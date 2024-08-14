#include "web_service.hpp"

namespace lexis {

WebService::WebService(QObject* parent) :
  QObject(parent)
{
  _manager = new QNetworkAccessManager(this);
  connect(_manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(onFinished(QNetworkReply*)));
}

void WebService::request(const QString& query) {
  _manager->get(QNetworkRequest(QUrl(query)));
}

}

