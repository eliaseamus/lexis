#pragma once

#include <QString>
#include <QVector>

#include <optional>

namespace lexis {

class TextEmbedding {
 public:
  static std::optional<QVector<float>> embed(const QString& languageCode, const QString& text,
                                             const QString& secondaryLanguageCode = {});
  static std::optional<double> similarity(const QString& languageCode, const QString& left,
                                          const QString& right,
                                          const QString& secondaryLanguageCode = {});
  static std::optional<double> centroidSimilarity(const QString& languageCode,
                                                  const QVector<float>& target,
                                                  const QVector<QString>& members,
                                                  const QString& secondaryLanguageCode = {});
};

}  // namespace lexis
