#pragma once

#include <QString>
#include <QVector>

#include <optional>

namespace lexis {

class TextEmbedding {
 public:
  static std::optional<QVector<float>> embed(const QString& languageCode, const QString& text);
  static std::optional<double> similarity(const QString& languageCode, const QString& left,
                                            const QString& right);
  static std::optional<double> centroidSimilarity(const QString& languageCode,
                                                    const QVector<float>& target,
                                                    const QVector<QString>& members);
};

}  // namespace lexis
