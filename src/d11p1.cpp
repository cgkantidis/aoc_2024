#include "fmt/core.h"
#include "libassert/assert.hpp"
#include <array>
#include <cstdint>
#include <fstream>
#include <ranges>
#include <string>
#include <string_view>
#include <unordered_set>
#include <utility>

#include "matrix.hpp"
#include "utility.hpp"

static constexpr auto UNINIT = std::numeric_limits<unsigned>::max();

namespace
{
void
tests();
std::uint64_t
num_of_stones(std::ranges::range auto &&lines, std::uint64_t num_blinks);
std::vector<std::uint64_t>
blink(std::vector<std::uint64_t> const &stones);
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

  fmt::println("{}", num_of_stones(lines, 25));
  return 0;
}

namespace
{
void
tests() {
  using namespace std::literals::string_view_literals;
  {
    auto const line = std::array{
        "0 1 10 99 999"sv,
    };
    ASSERT(num_of_stones(line, 1) == 7);
  }
  {
    auto const line = std::array{
        "125 17"sv,
    };
    ASSERT(num_of_stones(line, 6) == 22);
    ASSERT(num_of_stones(line, 25) == 55312);
  }
}

std::uint64_t
num_of_stones(std::ranges::range auto &&lines, std::uint64_t num_blinks) {
  std::vector<std::uint64_t> stones =
      split(lines[0]) | std::views::transform(str_to_int<std::uint64_t>)
      | std::ranges::to<std::vector<std::uint64_t>>();
  for (std::uint64_t blink_idx = 0; blink_idx < num_blinks; ++blink_idx) {
    stones = blink(stones);
  }
  return stones.size();
}

std::vector<std::uint64_t>
blink(std::vector<std::uint64_t> const &stones) {
  std::vector<std::uint64_t> next_stones;
  next_stones.reserve(stones.size() * 2);

  for (std::uint64_t const &stone : stones) {
    if (stone == 0) {
      next_stones.push_back(1);
      continue;
    }
    auto stone_str = std::to_string(stone);
    auto num_digits = stone_str.size();
    if (num_digits % 2 == 1) {
      next_stones.push_back(stone * 2024);
      continue;
    }
    next_stones.push_back(str_to_int<uint64_t>(stone_str.substr(0, num_digits / 2)));
    next_stones.push_back(str_to_int<uint64_t>(stone_str.substr(num_digits / 2)));
  }
  return next_stones;
}
} // namespace
