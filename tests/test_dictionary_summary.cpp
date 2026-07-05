#include <QTest>

#include "dictionary.hpp"
#include "dictionary_summary.hpp"

using lexis::Definition;
using lexis::Translation;

class DictionarySummaryTest : public QObject {
  Q_OBJECT

 private:
  Definition* makeDefinition(const QString& sourceText, const QString& translationText,
                             const QStringList& synonyms = {}, const QStringList& meanings = {}) {
    auto* definition = new Definition(this);
    definition->setText(sourceText);
    definition->setPartOfSpeech(QStringLiteral("noun"));
    auto* translation = new Translation(definition);
    translation->setText(translationText);
    translation->setSynonyms(synonyms);
    translation->setMeanings(meanings);
    definition->setTranslations({translation});
    return definition;
  }

 private slots:
  void buildsUniqueSummaryLines() {
    const QVector<Definition*> definitions = {
      makeDefinition("steer", "руль", {"wheel", "steering wheel"}, {"vehicle part"}),
      makeDefinition("wheel", "колесо", {"wheel"}, {"vehicle part"}),
    };

    const auto summary = lexis::buildDictionarySummary(definitions);
    const auto lines = summary.split('\n');

    QCOMPARE(lines.size(), 7);
    QCOMPARE(lines[0], QString("steer"));
    QCOMPARE(lines[1], QString("noun"));
    QCOMPARE(lines[2], QString("руль"));
    QCOMPARE(lines[3], QString("wheel"));
    QCOMPARE(lines[6], QString("колесо"));
  }

  void combinesSemanticContextWithoutDuplicates() {
    const auto combined = lexis::combineSemanticContext("fruit", "orange", "orange");
    QCOMPARE(combined, QString("fruit\norange"));
  }
};

QTEST_MAIN(DictionarySummaryTest)
#include "test_dictionary_summary.moc"
