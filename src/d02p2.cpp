#include <algorithm> // std::ranges::sort
#include <fmt/core.h>
#include <fstream>
#include <ranges>
#include <string>

#include <libassert/assert.hpp>

#include "utility.hpp"

bool
are_increasing(std::size_t a, std::size_t b) {
  return (a < b) && (b - a < 4);
}

bool
is_safe_int(std::ranges::range auto &&levels) {
  return std::ranges::all_of(std::views::slide(levels, 2), [](auto window) {
    return are_increasing(window[0], window[1]);
  });
}

bool
is_safe_int(std::ranges::range auto &&levels, std::size_t pivot) {
  auto left_end = std::next(levels.begin(), pivot);
  auto left = std::ranges::subrange(levels.begin(), left_end);
  auto right_begin = std::next(left_end, 1);
  auto right = std::ranges::subrange(right_begin, levels.end());

  if (left.empty()) {
    return is_safe_int(right);
  }
  if (right.empty()) {
    return is_safe_int(left);
  }
  return is_safe_int(left) && is_safe_int(right)
         && are_increasing(levels[pivot - 1], levels[pivot + 1]);
}

bool
is_safe(std::ranges::range auto &&levels) {
  for (std::size_t pivot = 0; pivot < levels.size(); ++pivot) {
    if (is_safe_int(levels, pivot)
        || is_safe_int(std::views::reverse(levels), pivot)) {
      return true;
    }
  }
  return false;
}

void
tests() {
  ASSERT(is_safe(std::vector<std::size_t>({{7, 6, 4, 2, 1}})));
  ASSERT(!is_safe(std::vector<std::size_t>({{1, 2, 7, 8, 9}})));
  ASSERT(!is_safe(std::vector<std::size_t>({{9, 7, 6, 2, 1}})));
  ASSERT(is_safe(std::vector<std::size_t>({{1, 3, 2, 4, 5}})));
  ASSERT(is_safe(std::vector<std::size_t>({{8, 6, 4, 4, 1}})));
  ASSERT(is_safe(std::vector<std::size_t>({{1, 3, 6, 7, 9}})));
  ASSERT(is_safe(std::vector<std::size_t>({{1, 5, 6}})));
  ASSERT(is_safe(std::vector<std::size_t>({{1, 2, 6}})));
  ASSERT(is_safe(std::vector<std::size_t>({{1, 2, 3, 4}})));
  ASSERT(is_safe(std::vector<std::size_t>({{1, 4, 7, 10}})));
  ASSERT(is_safe(std::vector<std::size_t>({{1, 5, 7, 10}})));
  ASSERT(is_safe(std::vector<std::size_t>({{5, 6, 5, 7}})));
}

int
main(int argc, char const *const *argv) {
  tests();

  auto args = std::span(argv, size_t(argc));
  if (args.size() != 2) {
    fmt::println(stderr, "usage: {} input.txt", args[0]);
    return 1;
  }

  std::ifstream infile(argv[1]);
  if (!infile.is_open()) {
    fmt::println(stderr, "couldn't open file {}", argv[1]);
    return 2;
  }

  std::size_t total{};
  for (std::string line; std::getline(infile, line);) {
    std::vector<std::string_view> tokens = split(line);
    std::vector<std::uint64_t> levels;
    std::ranges::transform(tokens, std::back_inserter(levels), str_to_int<std::uint64_t>);
    if (is_safe(levels)) {
      ++total;
    }
  }
  fmt::println("{}", total);
  return 0;
}
