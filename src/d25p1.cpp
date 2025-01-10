#include "BS_thread_pool.hpp"
#include "utility.hpp" // str_to_int
#include <array> // std::array
#include <cstdint> // std::uint64_t
#include <fmt/core.h> // fmt::print
#include <fmt/ranges.h> // fmt::print
#include <fstream> // std::ifstream
#include <libassert/assert.hpp> // ASSERT
#include <ranges> // std::views::enumerate
#include <string> // std::string
#include <vector> // std::vector

using ulong = std::uint64_t;
using byte = std::uint8_t;
using namespace std::literals::string_view_literals;

using heights = std::array<byte, 5>;

namespace
{
void
tests();
ulong
get_num_key_lock_pairs(std::ranges::range auto &&lines);
std::pair<std::vector<heights>, std::vector<heights>>
parse_keys_and_locks(std::ranges::range auto &&lines);
ulong
get_num_key_lock_pairs(std::vector<heights> const &keys,
                       std::vector<heights> const &locks);
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

  fmt::println("{}", get_num_key_lock_pairs(lines));
  return 0;
}

namespace
{
void
tests() {
  {
    auto const lines = std::array{
      "#####"sv,
      ".####"sv,
      ".####"sv,
      ".####"sv,
      ".#.#."sv,
      ".#..."sv,
      "....."sv,
      ""sv,
      "#####"sv,
      "##.##"sv,
      ".#.##"sv,
      "...##"sv,
      "...#."sv,
      "...#."sv,
      "....."sv,
      ""sv,
      "....."sv,
      "#...."sv,
      "#...."sv,
      "#...#"sv,
      "#.#.#"sv,
      "#.###"sv,
      "#####"sv,
      ""sv,
      "....."sv,
      "....."sv,
      "#.#.."sv,
      "###.."sv,
      "###.#"sv,
      "###.#"sv,
      "#####"sv,
      ""sv,
      "....."sv,
      "....."sv,
      "....."sv,
      "#...."sv,
      "#.#.."sv,
      "#.#.#"sv,
      "#####"sv,
    };
    ASSERT(get_num_key_lock_pairs(lines) == 3);
  }
}

ulong
get_num_key_lock_pairs(std::ranges::range auto &&lines) {
  auto [keys, locks] = parse_keys_and_locks(lines);
  return get_num_key_lock_pairs(keys, locks);
}

std::pair<std::vector<heights>, std::vector<heights>>
parse_keys_and_locks(std::ranges::range auto &&lines) {
  std::vector<heights> keys;
  std::vector<heights> locks;

  bool is_key{};
  heights h{};
  for (auto const &[idx, line] : std::views::enumerate(lines)) {
    if (idx % 8 == 0) {
      if (idx != 0) {
        if (is_key) {
          keys.emplace_back(h);
        } else {
          locks.emplace_back(h);
        }
        std::ranges::fill(h, 0);
      }

      // new beginning
      is_key = line[0] == '.';
    }
    if ((is_key && idx % 8 == 6) || (!is_key && idx % 8 == 0)) {
      continue;
    }
    for (auto const &[col, ch] : std::views::enumerate(line)) {
      if (ch == '#') {
        ++h[col];
      }
    }
  }
  // last item
  if (is_key) {
    keys.emplace_back(h);
  } else {
    locks.emplace_back(h);
  }
  return {keys, locks};
}

ulong
get_num_key_lock_pairs(std::vector<heights> const &keys,
                       std::vector<heights> const &locks) {
  ulong pairs{};
  for (auto const &key : keys) {
    for (auto const &lock : locks) {
      if (std::ranges::all_of(std::views::zip(key, lock), [](auto const &t) {
            return std::get<0>(t) + std::get<1>(t) <= 5;
          })) {
        ++pairs;
      }
    }
  }
  return pairs;
}
} // namespace
