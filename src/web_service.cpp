#include "web_service.hpp"

namespace lexis {

WebService::WebService(QObject* parent) :
  QObject(parent)
{
  _manager = new QNetworkAccessManager(this);
  connect(_manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(onFinished(QNetworkReply*)));
}

void WebService::get(const QString& query) {
  _manager->get(QNetworkRequest(QUrl(query)));
}

void WebService::get(const QNetworkRequest& request) {
  _manager->get(request);
}

void WebService::post(const QNetworkRequest& request, const QByteArray& body) {
  _manager->post(request, body);
}

}

