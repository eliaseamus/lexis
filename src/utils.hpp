#pragma once

#include <QString>
#include <ranges>
#include <utility>

class QTemporaryFile;

namespace lexis {

#define MAKE_STR(x) _MAKE_STR(x)
#define _MAKE_STR(x) #x

constexpr inline auto enumRange = [](auto front, auto back) {
  return std::views::iota(std::to_underlying(front), std::to_underlying(back) + 1) |
         std::views::transform([](auto e) {
           return decltype(front)(e);
         });
};

QString temporaryFileTemplate();
bool writeCompressedBlob(QTemporaryFile& file, const QByteArray& compressedBlob);

}  // namespace lexis
