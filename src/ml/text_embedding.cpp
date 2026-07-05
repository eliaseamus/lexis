#include "text_embedding.hpp"

#include <cmath>

#include "text_similarity.hpp"

namespace lexis {

EmbeddingVector TextEmbedding::embedText(const QString& languageCode, const QString& text) {
  QVector<EmbeddingVector> tokenVectors;
  for (const auto& token : tokenizeText(text)) {
    auto vector = EmbeddingLookup::lookup(languageCode, token);
    if (!vector.isEmpty()) {
      tokenVectors.append(std::move(vector));
    }
  }

  auto mean = meanVector(tokenVectors);
  normalize(mean);
  return mean;
}

double TextEmbedding::similarity(const EmbeddingVector& left, const EmbeddingVector& right) {
  if (left.isEmpty() || left.size() != right.size()) {
    return 0.0;
  }

  double dot = 0.0;
  for (int i = 0; i < left.size(); ++i) {
    dot += static_cast<double>(left[i]) * right[i];
  }
  if (dot < 0.0) {
    return 0.0;
  }
  return dot > 1.0 ? 1.0 : dot;
}

EmbeddingVector TextEmbedding::meanVector(const QVector<EmbeddingVector>& vectors) {
  if (vectors.isEmpty()) {
    return {};
  }

  EmbeddingVector mean(vectors.front().size(), 0.0F);
  for (const auto& vector : vectors) {
    if (vector.size() != mean.size()) {
      continue;
    }
    for (int i = 0; i < mean.size(); ++i) {
      mean[i] += vector[i];
    }
  }
  for (auto& component : mean) {
    component /= static_cast<float>(vectors.size());
  }
  return mean;
}

void TextEmbedding::normalize(EmbeddingVector& vector) {
  double squaredNorm = 0.0;
  for (const auto component : vector) {
    squaredNorm += static_cast<double>(component) * component;
  }
  if (squaredNorm <= 0.0) {
    vector.clear();
    return;
  }
  const auto norm = static_cast<float>(std::sqrt(squaredNorm));
  for (auto& component : vector) {
    component /= norm;
  }
}

}  // namespace lexis
