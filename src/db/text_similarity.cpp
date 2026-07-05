#include "text_similarity.hpp"

#include <QSet>

namespace lexis {

QStringList tokenizeText(const QString& text) {
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

double jaccardSimilarity(const QStringList& leftTokens, const QStringList& rightTokens) {
  if (leftTokens.isEmpty() || rightTokens.isEmpty()) {
    return 0.0;
  }

  QSet<QString> leftSet(leftTokens.begin(), leftTokens.end());
  QSet<QString> rightSet(rightTokens.begin(), rightTokens.end());
  int intersection = 0;
  for (const auto& token : leftSet) {
    if (rightSet.contains(token)) {
      ++intersection;
    }
  }

  const int unionSize = leftSet.size() + rightSet.size() - intersection;
  if (unionSize <= 0) {
    return 0.0;
  }
  return static_cast<double>(intersection) / unionSize;
}

double substringSimilarity(const QString& left, const QString& right) {
  const auto normalizedLeft = left.trimmed().toCaseFolded();
  const auto normalizedRight = right.trimmed().toCaseFolded();
  if (normalizedLeft.isEmpty() || normalizedRight.isEmpty()) {
    return 0.0;
  }
  if (normalizedLeft == normalizedRight) {
    return 1.0;
  }
  if (normalizedLeft.contains(normalizedRight) || normalizedRight.contains(normalizedLeft)) {
    return 0.75;
  }
  return 0.0;
}

double textSimilarity(const QString& left, const QString& right) {
  const auto jaccard = jaccardSimilarity(tokenizeText(left), tokenizeText(right));
  const auto substring = substringSimilarity(left, right);
  return jaccard >= substring ? jaccard : substring;
}

}  // namespace lexis
