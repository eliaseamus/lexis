#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSignalSpy>
#include <QTest>
#include <ranges>

#include "predictor.hpp"

class Predictor : public QObject {
  Q_OBJECT

 private:
  lexis::Predictor* _service = nullptr;

 private:
  QJsonDocument buildReply(bool endOfWord, int pos, QStringList textValues) {
    QJsonObject reply;
    reply["code"] = 200;
    reply["endOfWord"] = endOfWord;
    reply["pos"] = pos;

    QJsonArray text;
    for (const auto& value : textValues) {
      text.append(value);
    }
    reply["text"] = text;
    return QJsonDocument(reply);
  }

 private slots:
  void initTestCase() {
    _service = new lexis::Predictor(this);
  }

  void singleLetter() {
    _service->setQuery("d");
    auto textValues = QStringList() << "download" << "do" << "day" << "don't" << "data";

    QSignalSpy spy(_service, SIGNAL(predictionsReceived(QStringList)));
    _service->retrievePredictions(buildReply(true, -1, textValues));
    QCOMPARE(spy.count(), 1);
    auto args = spy.takeFirst();

    QVERIFY(args.at(0).toStringList() == textValues);
  }

  void halfOfWord() {
    QString query = "do";
    _service->setQuery(query);
    auto textValues = QStringList() << "download" << "don't" << "down" << "does" << "done";

    QSignalSpy spy(_service, SIGNAL(predictionsReceived(QStringList)));
    _service->retrievePredictions(buildReply(true, -2, textValues));
    QCOMPARE(spy.count(), 1);
    auto args = spy.takeFirst();

    QVERIFY(args.at(0).toStringList() == textValues);
  }

  void word() {
    QString query = "door";
    _service->setQuery(query);
    auto textValues = QStringList() << "and" << "to" << "of" << "i" << "is";

    QSignalSpy spy(_service, SIGNAL(predictionsReceived(QStringList)));
    _service->retrievePredictions(buildReply(true, 1, textValues));
    QCOMPARE(spy.count(), 1);
    auto args = spy.takeFirst();

    QStringList predictions;
    predictions.reserve(textValues.size());
    std::ranges::transform(textValues, std::back_inserter(predictions),
                           [&query](const QString& value) {
                             return QString("%1 %2").arg(query, value);
                           });
    QVERIFY(args.at(0).toStringList() == predictions);
  }

  void wordWithASpaceAtTheEnd() {
    QString query = "door ";
    _service->setQuery(query);
    auto textValues = QStringList() << "and" << "to" << "of" << "i" << "is";

    QSignalSpy spy(_service, SIGNAL(predictionsReceived(QStringList)));
    _service->retrievePredictions(buildReply(true, 0, textValues));
    QCOMPARE(spy.count(), 1);
    auto args = spy.takeFirst();

    QStringList predictions;
    predictions.reserve(textValues.size());
    std::ranges::transform(textValues, std::back_inserter(predictions),
                           [&query](const QString& value) {
                             return query + value;
                           });
    QVERIFY(args.at(0).toStringList() == predictions);
  }

  void notTheEndOfTheWord() {
    QString query = "doorst";
    _service->setQuery(query);
    auto textValues = QStringList() << "doorstep" << "doorsteps" << "doorstop" << "doors"
                                    << "doors to";

    QSignalSpy spy(_service, SIGNAL(predictionsReceived(QStringList)));
    _service->retrievePredictions(buildReply(false, -6, textValues));
    QCOMPARE(spy.count(), 1);
    auto args = spy.takeFirst();

    QVERIFY(args.at(0).toStringList() == textValues);
  }
};

QTEST_MAIN(Predictor)
#include "test_predictor.moc"
