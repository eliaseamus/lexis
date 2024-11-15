#pragma once

#include <ranges>
#include <utility>

#include <QString>

#define MAKE_STR(x) _MAKE_STR(x)
#define _MAKE_STR(x) #x

namespace lexis {

constexpr inline auto enumRange = [](auto front, auto back) {
  return std::views::iota(std::to_underlying(front), std::to_underlying(back) + 1) |
         std::views::transform([](auto e) {return decltype(front)(e);});
};

QString temporaryFileTemplate();

}
