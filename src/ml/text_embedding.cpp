#include "text_embedding.hpp"

#include "embedding_lookup.hpp"

namespace lexis {

namespace {

QStringList tokenize(const QString& text) {
  QStringList tokens;
  QString current;
  for (const auto character : text.toCaseFolded()) {
    if (character.isLetterOrNumber()) {
      current.append(character);
      continue;
    }
    if (!current.isEmpty()) {
      tokens.append(current);
      current.clear();
    }
  }
  if (!current.isEmpty()) {
    tokens.append(current);
  }
  return tokens;
}

std::optional<QVector<float>> averageVectors(const QVector<QVector<float>>& vectors) {
  if (vectors.isEmpty()) {
    return std::nullopt;
  }

  QVector<float> sum = vectors.first();
  for (int index = 1; index < vectors.size(); ++index) {
    for (int dimension = 0; dimension < sum.size(); ++dimension) {
      sum[dimension] += vectors[index][dimension];
    }
  }

  const float scale = 1.0F / static_cast<float>(vectors.size());
  for (float& value : sum) {
    value *= scale;
  }
  return sum;
}

std::optional<QVector<float>> lookupToken(const QString& languageCode,
                                          const QString& secondaryLanguageCode,
                                          const QString& token) {
  const auto primary = EmbeddingLookup::lookup(languageCode, token);
  if (primary.found) {
    return primary.values;
  }
  if (secondaryLanguageCode.isEmpty() || secondaryLanguageCode == languageCode) {
    return std::nullopt;
  }

  const auto secondary = EmbeddingLookup::lookup(secondaryLanguageCode, token);
  if (!secondary.found) {
    return std::nullopt;
  }
  return secondary.values;
}

}  // namespace

std::optional<QVector<float>> TextEmbedding::embed(const QString& languageCode,
                                                   const QString& text,
                                                   const QString& secondaryLanguageCode) {
  if (!EmbeddingLookup::isOpen() || languageCode.isEmpty()) {
    return std::nullopt;
  }

  QVector<QVector<float>> vectors;
  for (const auto& token : tokenize(text)) {
    if (const auto vector = lookupToken(languageCode, secondaryLanguageCode, token)) {
      vectors.append(*vector);
    }
  }

  return averageVectors(vectors);
}

std::optional<double> TextEmbedding::similarity(const QString& languageCode, const QString& left,
                                                const QString& right,
                                                const QString& secondaryLanguageCode) {
  const auto leftVector = embed(languageCode, left, secondaryLanguageCode);
  const auto rightVector = embed(languageCode, right, secondaryLanguageCode);
  if (!leftVector.has_value() || !rightVector.has_value()) {
    return std::nullopt;
  }

  const auto cosine = EmbeddingLookup::cosineSimilarity(*leftVector, *rightVector);
  return cosine > 0.0 ? cosine : 0.0;
}

std::optional<double> TextEmbedding::centroidSimilarity(
  const QString& languageCode, const QVector<float>& target, const QVector<QString>& members,
  const QString& secondaryLanguageCode) {
  if (!EmbeddingLookup::isOpen() || members.isEmpty()) {
    return std::nullopt;
  }

  QVector<QVector<float>> memberVectors;
  memberVectors.reserve(members.size());
  for (const auto& member : members) {
    if (const auto vector = embed(languageCode, member, secondaryLanguageCode)) {
      memberVectors.append(*vector);
    }
  }

  const auto centroid = averageVectors(memberVectors);
  if (!centroid.has_value()) {
    return std::nullopt;
  }

  const auto cosine = EmbeddingLookup::cosineSimilarity(target, *centroid);
  return cosine > 0.0 ? cosine : 0.0;
}

}  // namespace lexis
