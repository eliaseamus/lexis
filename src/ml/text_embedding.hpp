#pragma once

#include <QString>

#include "embedding_lookup.hpp"

namespace lexis {

// Semantic text utilities built on top of EmbeddingLookup.
class TextEmbedding {
 public:
  // Mean of the word vectors of all recognized tokens, re-normalized to unit
  // length. Empty when no token is covered by the embedding vocabulary.
  static EmbeddingVector embedText(const QString& languageCode, const QString& text);

  // Cosine similarity of two unit vectors, clamped to [0, 1]
  // (negative similarities carry no signal for grouping).
  static double similarity(const EmbeddingVector& left, const EmbeddingVector& right);

  static EmbeddingVector meanVector(const QVector<EmbeddingVector>& vectors);
  static void normalize(EmbeddingVector& vector);
};

}  // namespace lexis
