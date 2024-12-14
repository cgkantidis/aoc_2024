#include "fmt/core.h"
#include "libassert/assert.hpp"
#include <array>
#include <cstdint>
#include <fstream>
#include <map>
#include <ranges>
#include <string>
#include <string_view>
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
std::uint64_t
blink(std::uint64_t stone, std::uint64_t num_blinks);
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

  fmt::println("{}", num_of_stones(lines, 75));
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
  return std::ranges::fold_left(
      stones,
      0ULL,
      [num_blinks](std::uint64_t const &prev, std::uint64_t const &stone) {
        return prev + blink(stone, num_blinks);
      });
}

std::map<std::pair<std::uint64_t, std::uint64_t>, std::uint64_t> cache;

std::uint64_t
blink(std::uint64_t stone, std::uint64_t num_blinks) {
  auto key = std::make_pair(stone, num_blinks);
  auto find_it = cache.find(key);
  if (find_it != cache.end()) {
    return find_it->second;
  }

  if (num_blinks == 0) {
    cache[key] = 1;
    return 1;
  }
  if (stone == 0) {
    cache[key] = blink(1, num_blinks - 1);
    return cache[key];
  }
  auto stone_str = std::to_string(stone);
  auto num_digits = stone_str.size();
  if (num_digits % 2 == 1) {
    cache[key] = blink(stone * 2024, num_blinks - 1);
    return cache[key];
  }
  cache[key] = blink(str_to_int<uint64_t>(stone_str.substr(0, num_digits / 2)),
                     num_blinks - 1)
               + blink(str_to_int<uint64_t>(stone_str.substr(num_digits / 2)),
                       num_blinks - 1);
  return cache[key];
}
} // namespace
