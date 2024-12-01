#include <fmt/core.h>
#include <fstream>
#include <string>
#include <unordered_map> // std::unordered_map

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
  std::unordered_map<std::uint64_t, std::uint64_t> frequency;
  std::string line;
  while (std::getline(infile, line)) {
    auto tokens = split(line);
    left_list.emplace_back(str_to_int(tokens[0]));
    std::size_t right{str_to_int(tokens[1])};
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
  fmt::println("{}", total);
  return 0;
}

