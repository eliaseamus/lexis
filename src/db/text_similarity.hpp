#pragma once

#include <QString>
#include <QStringList>

namespace lexis {

QStringList tokenizeText(const QString& text);
double jaccardSimilarity(const QStringList& leftTokens, const QStringList& rightTokens);
double substringSimilarity(const QString& left, const QString& right);
double textSimilarity(const QString& left, const QString& right);

}  // namespace lexis
