#include <algorithm> // std::ranges::sort
#include <fmt/core.h>
#include <fmt/ranges.h>
#include <fstream>
#include <string>
#include <ranges>

#include "utility.hpp"

bool
are_increasing(std::size_t a, std::size_t b) {
  return (a < b) && (b - a < 4);
}

bool
is_safe_int(std::vector<std::size_t> const &levels) {
  bool error_found{false};
  std::size_t skip_next{0};
  for (auto window : std::ranges::slide_view(levels, 3)) {
    //fmt::println("{} {} {}", window[0], window[1], window[2]);
    if (skip_next != 0) {
      //fmt::println("skip");
      --skip_next;
      continue;
    }
    bool inc_01 = are_increasing(window[0], window[1]);
    bool inc_12 = are_increasing(window[1], window[2]);
    if (inc_01 && inc_12) {
      continue;
    }
    if (!(inc_01 || inc_12)) {
      //fmt::println("false *");
      return false;
    }
    if (error_found) {
      //fmt::println("false **");
      return false;
    }
    //fmt::print("*");
    if (inc_12) {
      error_found = true;
      //fmt::println("[{}]{}{}", window[0], window[1], window[2]);
      // we're skipping the window[0]
      continue;
    }
    bool inc_02 = are_increasing(window[0], window[2]);
    if (inc_02) {
      error_found = true;
      //fmt::println("{}[{}]{}", window[0], window[1], window[2]);
      // we're skipping the window[1]
      skip_next = 1;
      continue;
    }
  }
  //fmt::println("true");
  return true;
}
//bool
//is_safe_int(std::vector<std::size_t> const &levels) {
//  for (auto window : std::ranges::slide_view(levels, 2)) {
//    if (!are_increasing(window[0], window[1])) {
//      return false;
//    }
//  }
//  return true;
//}

bool
is_safe(std::vector<std::size_t> &levels) {
  if (is_safe_int(levels)) {
    return true;
  }
  std::ranges::reverse(levels);
  return is_safe_int(levels);
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
    std::vector<std::string_view> tokens = split(line);
    std::vector<std::uint64_t> levels;
    std::ranges::transform(tokens, std::back_inserter(levels), str_to_int);
    if (is_safe(levels)) {
      ++total;
    } else {
      fmt::println("{}", levels);
    }
  }
  fmt::println("{}", total);
  return 0;
}

