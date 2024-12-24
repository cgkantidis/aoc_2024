#include <array>
#include <cstdint>
#include <deque> // deque
#include <fmt/core.h>
#include <fstream>
#include <libassert/assert.hpp>
#include <ranges>
#include <scn/scan.h>
#include <string>
#include <string_view>
#include <unordered_map> // unordered_map
#include <utility>

#include "matrix.hpp"
#include "utility.hpp"

namespace
{
void
tests();
std::uint64_t
num_possible_designs(std::ranges::range auto &&lines);
std::pair<std::vector<std::string>, std::vector<std::string>>
parse_towels_and_patterns(std::ranges::range auto &&lines);
std::uint64_t
num_possible_designs(std::vector<std::string> const &towels,
                     std::vector<std::string> const &patterns);
bool
is_pattern_possible(std::vector<std::string> const &towels,
                    std::string const &pattern,
                    std::size_t beg_idx,
                    std::vector<std::string> &matches);
} // namespace

int
main(int argc, char const *const *argv) {
  tests();

  auto args = std::span(argv, size_t(argc));
  if (args.size() != 2) {
    fmt::println(stderr, "usage: {} input.txt", args[0]);
    return 1;
  }

  std::ifstream infile(args[1]);
  if (!infile.is_open()) {
    fmt::println(stderr, "couldn't open file {}", args[1]);
    return 2;
  }

  std::vector<std::string> lines;
  for (std::string line; std::getline(infile, line);) {
    lines.emplace_back(std::move(line));
  }

  fmt::println("{}", num_possible_designs(lines));
  return 0;
}

namespace
{
void
tests() {
  using namespace std::literals::string_view_literals;
  {
    auto const lines = std::array{
        "r, wr, b, g, bwu, rb, gb, br"sv,
        ""sv,
        "brwrr"sv,
        "bggr"sv,
        "gbbr"sv,
        "rrbgbr"sv,
        "ubwu"sv,
        "bwurrg"sv,
        "brgr"sv,
        "bbrgwb"sv,
    };
    ASSERT(num_possible_designs(lines) == 6);
  }
}

std::uint64_t
num_possible_designs(std::ranges::range auto &&lines) {
  auto [towels, patterns] = parse_towels_and_patterns(lines);
  return num_possible_designs(towels, patterns);
}

std::pair<std::vector<std::string>, std::vector<std::string>>
parse_towels_and_patterns(std::ranges::range auto &&lines) {
  auto towels =
      split(lines[0], ", ") | std::ranges::to<std::vector<std::string>>();
  std::ranges::sort(towels, [](std::string const &lhs, std::string const &rhs) {
    if (lhs.size() > rhs.size()) {
      return true;
    }
    if (lhs.size() < rhs.size()) {
      return false;
    }
    return lhs < rhs;
  });
  auto patterns =
      std::vector<std::string>(std::next(lines.begin(), 2), lines.end());

  return {towels, patterns};
}

std::uint64_t
num_possible_designs(std::vector<std::string> const &towels,
                     std::vector<std::string> const &patterns) {
  std::uint64_t num_possible{};
  for (auto const &pattern : patterns) {
    std::vector<std::string> matches;
    fmt::print("{}: ", pattern);
    if (is_pattern_possible(towels, pattern, 0, matches)) {
      bool is_first = true;
      for (std::string const &match : matches | std::views::reverse) {
        if (is_first) {
          is_first = false;
          fmt::print("{}", match);
        } else {
          fmt::print(",{}", match);
        }
      }
      fmt::println("");
      ++num_possible;
    } else {
      fmt::println("impossible");
    }
  }

  return num_possible;
}

bool
is_pattern_possible(std::vector<std::string> const &towels,
                    std::string const &pattern,
                    std::size_t beg_idx,
                    std::vector<std::string> &matches) {
  if (beg_idx == pattern.size()) {
    return true;
  }
  for (std::string const &towel : towels) {
    if (pattern.substr(beg_idx, towel.size()) != towel) {
      continue;
    }
    if (is_pattern_possible(towels, pattern, beg_idx + towel.size(), matches)) {
      matches.emplace_back(towel);
      return true;
    }
  }

  return false;
}
} // namespace
