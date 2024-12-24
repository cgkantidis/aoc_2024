#include <algorithm> // std::sort
#include <array> // std::array
#include <cstdint> // std::uint64_t
#include <fmt/core.h> // fmt::print
#include <fstream> // std::ifstream
#include <libassert/assert.hpp> // ASSERT
#include <ranges> // std::span
#include <string> // std::string
#include <unordered_map> // std::unordered_map
#include <vector> // std::vector

#include "matrix.hpp" // Matrix
#include "utility.hpp" // split

namespace
{
void
tests();
std::uint64_t
num_possible_ways(std::ranges::range auto &&lines);
std::pair<std::vector<std::string>, std::vector<std::string>>
parse_towels_and_patterns(std::ranges::range auto &&lines);
std::uint64_t
find_all_ways(std::vector<std::string> const &towels,
              std::string const &pattern);
Matrix<std::uint64_t>
build_adj_matrix(std::vector<std::string> const &towels,
                 std::string const &pattern);
std::uint64_t
num_ways_from_to_end(std::uint64_t from,
                     Matrix<std::uint64_t> const &adj_matrix,
                     std::unordered_map<std::uint64_t, std::uint64_t> &map);
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

  fmt::println("{}", num_possible_ways(lines));
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
    ASSERT(num_possible_ways(lines) == 16);
  }
}

std::uint64_t
num_possible_ways(std::ranges::range auto &&lines) {
  auto [towels, patterns] = parse_towels_and_patterns(lines);
  return std::ranges::fold_left(
      patterns,
      0ULL,
      [&towels](std::uint64_t const &prev, std::string const &pattern) {
        return prev + find_all_ways(towels, pattern);
      });
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
find_all_ways(std::vector<std::string> const &towels,
              std::string const &pattern) {
  auto const adj_matrix = build_adj_matrix(towels, pattern);

  std::unordered_map<std::uint64_t, std::uint64_t> map;
  map[pattern.size()] = 1; // terminating condition
  return num_ways_from_to_end(0, adj_matrix, map);
}

Matrix<std::uint64_t>
build_adj_matrix(std::vector<std::string> const &towels,
                 std::string const &pattern) {
  Matrix<std::uint64_t> adj_matrix(pattern.size(), pattern.size() + 1, 0);
  for (std::size_t idx = 0; idx < pattern.size(); ++idx) {
    for (std::string const &towel : towels) {
      if (pattern.substr(idx, towel.size()) == towel) {
        ++adj_matrix(idx, idx + towel.size());
      }
    }
  }
  return adj_matrix;
}

std::uint64_t
num_ways_from_to_end(std::uint64_t from,
                     Matrix<std::uint64_t> const &adj_matrix,
                     std::unordered_map<std::uint64_t, std::uint64_t> &map) {
  auto find_it = map.find(from);
  if (find_it != map.end()) {
    return find_it->second;
  }

  std::uint64_t num_ways{};
  for (std::size_t to{from + 1}; to < adj_matrix.cols(); ++to) {
    if (adj_matrix(from, to) == 1) {
      num_ways += num_ways_from_to_end(to, adj_matrix, map);
    }
  }
  return map.emplace(from, num_ways).first->second;
}
} // namespace
