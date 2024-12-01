#include <fmt/core.h>
#include <fstream>
#include <ranges>
#include <string>
#include <algorithm> // std::ranges::sort

#include "utility.hpp"

int main(int argc, char const * const *argv) {
  if (argc != 2) {
    fmt::println(stderr, "usage: {} input.txt", argv[0]);
    return 1;
  }

  std::ifstream infile(argv[1]);
  if (!infile.is_open()) {
    fmt::println(stderr, "couldn't open file {}", argv[1]);
    return 2;
  }

  std::vector<std::size_t> left_list;
  std::vector<std::size_t> right_list;
  std::string line;
  while (std::getline(infile, line)) {
    auto tokens = split(line);
    left_list.emplace_back(str_to_int(tokens[0]));
    right_list.emplace_back(str_to_int(tokens[1]));
  }

  std::ranges::sort(left_list);
  std::ranges::sort(right_list);

  std::uint64_t total{};
  for (auto const &[left, right] : std::views::zip(left_list, right_list)) {
    total += left > right ? left - right : right - left;
  }
  fmt::println("{}", total);
  return 0;
}
