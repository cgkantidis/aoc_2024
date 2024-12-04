#include <algorithm> // std::ranges::sort
#include <fmt/core.h>
#include <fstream>
#include <ranges>
#include <regex>
#include <string>

#include <libassert/assert.hpp>

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

std::uint64_t
collect_all_muls(std::string corrupted, bool &enabled) {
  std::uint64_t total{};
  std::regex mul_regex(R"(mul\((\d+),(\d+)\)|do\(\)|don't\(\))");
  for (std::smatch sm; std::regex_search(corrupted, sm, mul_regex);) {
    if (sm[0].str() == "do()") {
      enabled = true;
      corrupted = sm.suffix();
      continue;
    }
    if (sm[0].str() == "don't()") {
      enabled = false;
      corrupted = sm.suffix();
      continue;
    }
    if (!enabled) {
      corrupted = sm.suffix();
      continue;
    }
    auto factor1 = sm[1].str();
    auto factor2 = sm[2].str();
    std::uint64_t factor1_num{};
    std::uint64_t factor2_num{};
    std::from_chars(factor1.c_str(),
                    factor1.c_str() + factor1.size(),
                    factor1_num);
    std::from_chars(factor2.c_str(),
                    factor2.c_str() + factor2.size(),
                    factor2_num);
    total += factor1_num * factor2_num;
    corrupted = sm.suffix();
  }
  return total;
}

void
tests() {
  bool enabled{true};
  ASSERT(
      collect_all_muls(
          "xmul(2,4)&mul[3,7]!^don't()_mul(5,5)+mul(32,64](mul(11,8)undo()?mul(8,5))",
          enabled)
      == 48);
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

  std::uint64_t total{};
  bool enabled{true};
  for (std::string line; std::getline(infile, line);) {
    total += collect_all_muls(line, enabled);
  }
  fmt::println("{}", total);
  return 0;
}
