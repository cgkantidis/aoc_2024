#include <cstdint> // std::uint64_t
#include <fmt/core.h> // fmt::println
#include <fstream> // std::ifstream
#include <ranges> // std::ranges::range
#include <string> // std::string
#include <unordered_map> // std::unordered_map
#include <vector> // std::vector

#include "utility.hpp" // split

namespace
{
void
tests();
std::uint64_t
get_similarity_score(std::ranges::range auto &&lines);
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

  fmt::println("{}", get_similarity_score(lines));
  return 0;
}

namespace
{
void
tests() {
  using namespace std::literals::string_view_literals;
  {
    auto const lines = std::array{
        "3   4"sv,
        "4   3"sv,
        "2   5"sv,
        "1   3"sv,
        "3   9"sv,
        "3   3"sv,
    };
    ASSERT(get_similarity_score(lines) == 31);
  }
}

std::uint64_t
get_similarity_score(std::ranges::range auto &&lines) {
  std::vector<std::size_t> left_list;
  std::unordered_map<std::uint64_t, std::uint64_t> frequency;
  for (auto const &line : lines) {
    auto tokens = split(line);
    left_list.emplace_back(str_to_int<std::uint64_t>(tokens[0]));
    std::size_t right{str_to_int<std::uint64_t>(tokens[1])};
    auto [it, is_new] = frequency.emplace(right, 1);
    if (!is_new) {
      ++it->second;
    }
  }

  std::uint64_t total{};
  for (auto const &left : left_list) {
    auto it = frequency.find(left);
    if (it != frequency.end()) {
      total += left * it->second;
    }
  }
  return total;
}
} // namespace
