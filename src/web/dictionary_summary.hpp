#pragma once

#include <QString>
#include <QVector>

namespace lexis {

class Definition;

QString buildDictionarySummary(const QVector<Definition*>& definitions);
QString combineSemanticContext(const QString& meaning, const QString& dictionarySummary,
                               const QString& cachedTranslation = {});

}  // namespace lexis
