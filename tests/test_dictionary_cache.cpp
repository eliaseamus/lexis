#include <QTest>

#include "dictionary.hpp"

using lexis::Definition;

class DictionaryCache : public QObject {
  Q_OBJECT

 private:
  lexis::DictionaryCache* _cache = nullptr;
  Definition* _defFoo = nullptr;
  Definition* _defBar = nullptr;
  Definition* _defBaz = nullptr;

 private slots:
  void initTestCase() {
    _cache = new lexis::DictionaryCache(2, this);

    _defFoo = new Definition(this);
    _defFoo->setText("foo");

    _defBar = new Definition(this);
    _defBar->setText("bar");

    _defBaz = new Definition(this);
    _defBaz->setText("baz");
  }

  void cleanup() {
    _cache->clear();
  }

  void lastAddedGoesOnTop() {
    QString language = "en-ru";
    _cache->addDefinitions({_defFoo}, language);
    QVERIFY(_cache->getCache().first().query == _defFoo->text());

    _cache->addDefinitions({_defBar}, language);
    QVERIFY(_cache->getCache().first().query == _defBar->text());
  }

  void lastAccessedGoesOnTop() {
    QString language = "en-ru";
    _cache->addDefinitions({_defFoo}, language);
    _cache->addDefinitions({_defBar}, language);
    QVERIFY(_cache->getDefinitions(_defFoo->text(), language) != std::nullopt);
    QVERIFY(_cache->getCache().first().query == _defFoo->text());
  }

  void leastRecentlyUsedIsRemoved() {
    QString language = "en-ru";
    _cache->addDefinitions({_defFoo}, language);
    _cache->addDefinitions({_defBar}, language);
    _cache->addDefinitions({_defBaz}, language);
    QVERIFY(_cache->getDefinitions(_defFoo->text(), language) == std::nullopt);
  }

  void cacheIsCaseInsensitive() {
    QString language = "en-ru";
    _cache->addDefinitions({_defFoo}, language);
    QVERIFY(_cache->getDefinitions(_defFoo->text().toUpper(), language) != std::nullopt);
  }

  void interfaceLanguageChangeCausesCacheMiss() {
    QString language = "en-ru";
    _cache->addDefinitions({_defFoo}, language);
    QVERIFY(_cache->getDefinitions(_defFoo->text(), language) != std::nullopt);
    language = "en-en";
    QVERIFY(_cache->getDefinitions(_defFoo->text(), language) == std::nullopt);
  }
};

QTEST_MAIN(DictionaryCache)
#include "test_dictionary_cache.moc"
