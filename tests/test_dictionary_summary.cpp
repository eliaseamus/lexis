#include <QTest>

#include "dictionary.hpp"
#include "dictionary_summary.hpp"

using lexis::Definition;
using lexis::Translation;

class DictionarySummaryTest : public QObject {
  Q_OBJECT

 private:
  Definition* makeDefinition(const QString& translationText, const QStringList& synonyms = {},
                             const QStringList& meanings = {}) {
    auto* definition = new Definition(this);
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
      makeDefinition("руль", {"wheel", "steering wheel"}, {"vehicle part"}),
      makeDefinition("колесо", {"wheel"}, {"vehicle part"}),
    };

    const auto summary = lexis::buildDictionarySummary(definitions);
    const auto lines = summary.split('\n');

    QCOMPARE(lines.size(), 5);
    QCOMPARE(lines[0], QString("руль"));
    QCOMPARE(lines[1], QString("wheel"));
    QCOMPARE(lines[2], QString("steering wheel"));
    QCOMPARE(lines[3], QString("vehicle part"));
    QCOMPARE(lines[4], QString("колесо"));
  }

  void combinesSemanticContextWithoutDuplicates() {
    const auto combined = lexis::combineSemanticContext("fruit", "orange", "orange");
    QCOMPARE(combined, QString("fruit\norange"));
  }
};

QTEST_MAIN(DictionarySummaryTest)
#include "test_dictionary_summary.moc"
