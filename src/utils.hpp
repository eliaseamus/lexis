#pragma once

#include <QUrl>
#include <ranges>

#define MAKE_STR(x) _MAKE_STR(x)
#define _MAKE_STR(x) #x

namespace lexis {

QByteArray readFile(const QUrl& url);

constexpr inline auto enumRange = [](auto front, auto back) {
  return std::views::iota(std::to_underlying(front), std::to_underlying(back) + 1) |
         std::views::transform([](auto e) {return decltype(front)(e);});
};

}
