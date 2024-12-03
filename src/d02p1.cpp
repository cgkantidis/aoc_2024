#include <algorithm> // std::ranges::sort
#include <fmt/core.h>
#include <fmt/ranges.h>
#include <fstream>
#include <ranges>
#include <string>

#include "utility.hpp"

bool
is_safe_int(std::ranges::input_range auto &&range) {
  return std::ranges::all_of(range | std::views::slide(2), [](auto const &window) {
    return window[0] < window[1] && window[1] - window[0] < 4;
  });
}

bool
is_safe(std::ranges::input_range auto &&range) {
  return is_safe_int(range) || is_safe_int(std::ranges::reverse_view(range));
}

int
main(int argc, char const *const *argv) {
  if (argc != 2) {
    fmt::println(stderr, "usage: {} input.txt", argv[0]);
    return 1;
  }

  std::ifstream infile(argv[1]);
  if (!infile.is_open()) {
    fmt::println(stderr, "couldn't open file {}", argv[1]);
    return 2;
  }

  std::size_t total{};
  for (std::string line; std::getline(infile, line);) {
    auto tokens = split(line);
    if (is_safe(tokens | std::views::transform(str_to_int))) {
      ++total;
    }
  }
  fmt::println("{}", total);
  return 0;
}
