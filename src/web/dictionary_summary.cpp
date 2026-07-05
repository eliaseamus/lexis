#include "dictionary_summary.hpp"

#include <QSet>

#include "dictionary.hpp"

namespace lexis {

namespace {

void appendUnique(QStringList* lines, QSet<QString>* seen, const QString& value) {
  const auto trimmed = value.trimmed();
  if (trimmed.isEmpty() || seen->contains(trimmed)) {
    return;
  }
  seen->insert(trimmed);
  lines->append(trimmed);
}

}  // namespace

QString buildDictionarySummary(const QVector<Definition*>& definitions) {
  QStringList lines;
  QSet<QString> seen;

  for (const auto* definition : definitions) {
    if (!definition) {
      continue;
    }

    appendUnique(&lines, &seen, definition->text());
    appendUnique(&lines, &seen, definition->partOfSpeech());

    for (const auto* translation : definition->translations()) {
      if (!translation) {
        continue;
      }

      appendUnique(&lines, &seen, translation->text());
      for (const auto& synonym : translation->synonyms()) {
        appendUnique(&lines, &seen, synonym);
      }
      for (const auto& gloss : translation->meanings()) {
        appendUnique(&lines, &seen, gloss);
      }
    }
  }

  return lines.join(QStringLiteral("\n"));
}

QString combineSemanticContext(const QString& meaning, const QString& dictionarySummary,
                               const QString& cachedTranslation) {
  QStringList parts;
  QSet<QString> seen;

  const auto appendPart = [&](const QString& value) {
    const auto trimmed = value.trimmed();
    if (trimmed.isEmpty() || seen.contains(trimmed)) {
      return;
    }
    seen.insert(trimmed);
    parts.append(trimmed);
  };

  appendPart(meaning);
  appendPart(dictionarySummary);
  appendPart(cachedTranslation);
  return parts.join(QStringLiteral("\n"));
}

}  // namespace lexis
