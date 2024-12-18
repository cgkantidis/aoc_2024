#include <array>
#include <cstdint>
#include <fmt/core.h>
#include <fstream>
#include <libassert/assert.hpp>
#include <queue>
#include <ranges>
#include <scn/scan.h>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>

#include "matrix.hpp"
#include "utility.hpp"

struct Button
{
  std::uint64_t step_x;
  std::uint64_t step_y;
  std::uint64_t cost;
};

struct Game
{
  Button button_A;
  Button button_B;
  std::uint64_t prize_x;
  std::uint64_t prize_y;
};

struct Position
{
  std::uint64_t x;
  std::uint64_t y;
};

/// we need the operator==() to resolve hash collisions
bool
operator==(Position const &lhs, Position const &rhs) {
  return lhs.x == rhs.x && lhs.y == rhs.y;
}

/// we can use the hash_combine variadic-template function, to combine multiple
/// hashes into a single one
template <typename T, typename... Rest>
constexpr void
hash_combine(std::size_t &seed, T const &val, Rest const &...rest) {
  constexpr size_t hash_mask{0x9e3779b9};
  constexpr size_t lsh{6};
  constexpr size_t rsh{2};
  seed ^= std::hash<T>{}(val) + hash_mask + (seed << lsh) + (seed >> rsh);
  (hash_combine(seed, rest), ...);
}

/// custom specialization of std::hash injected in namespace std
template <>
struct std::hash<Position>
{
  std::size_t
  operator()(Position const &s) const noexcept {
    std::size_t h1 = std::hash<std::size_t>{}(s.x);
    std::size_t h2 = std::hash<std::size_t>{}(s.y);

    std::size_t ret_val = 0;
    hash_combine(ret_val, h1, h2);
    return ret_val;
  }
};

namespace
{
void
tests();
std::uint64_t
min_num_tokens(std::ranges::range auto &&lines);
std::vector<Game>
generate_games(std::ranges::range auto &&lines);
std::uint64_t
min_num_tokens(Game const &game);
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

  fmt::println("{}", min_num_tokens(lines));
  return 0;
}

// step_x_a * N + step_x_b * M = VAL1;
// step_y_a * N + step_y_b * M = VAL2;
// step_y_b * M = VAL2 - step_y_a * N
// M = (VAL2 - step_y_a * N) / step_y_b
//
// step_x_a * N + step_x_b * ((VAL2 - step_y_a * M) / step_y_b) = VAL1
// step_x_a * N = VAL1 - step_x_b * ((VAL2 - step_y_a * M) / step_y_b)
// N = (VAL1 - step_x_b * ((VAL2 - step_y_a * M) / step_y_b)) / step_x_a

namespace
{
void
tests() {
  using namespace std::literals::string_view_literals;
  {
    auto const lines = std::array{
        "Button A: X+94, Y+34"sv,
        "Button B: X+22, Y+67"sv,
        "Prize: X=8400, Y=5400"sv,
    };
    ASSERT(min_num_tokens(lines) == 0);
  }
  {
    auto const lines = std::array{
        "Button A: X+26, Y+66"sv,
        "Button B: X+67, Y+21"sv,
        "Prize: X=12748, Y=12176"sv,
    };
    ASSERT(min_num_tokens(lines) == 459236326669);
  }
  {
    auto const line = std::array{
        "Button A: X+17, Y+86"sv,
        "Button B: X+84, Y+37"sv,
        "Prize: X=7870, Y=6450"sv,
    };
    ASSERT(min_num_tokens(line) == 0);
  }
  {
    auto const line = std::array{
        "Button A: X+69, Y+23"sv,
        "Button B: X+27, Y+71"sv,
        "Prize: X=18641, Y=10279"sv,
    };
    ASSERT(min_num_tokens(line) == 416082282239);
  }
  {
    auto const line = std::array{
        "Button A: X+94, Y+34"sv,
        "Button B: X+22, Y+67"sv,
        "Prize: X=8400, Y=5400"sv,
        ""sv,
        "Button A: X+26, Y+66"sv,
        "Button B: X+67, Y+21"sv,
        "Prize: X=12748, Y=12176"sv,
        ""sv,
        "Button A: X+17, Y+86"sv,
        "Button B: X+84, Y+37"sv,
        "Prize: X=7870, Y=6450"sv,
        ""sv,
        "Button A: X+69, Y+23"sv,
        "Button B: X+27, Y+71"sv,
        "Prize: X=18641, Y=10279"sv,
    };
    ASSERT(min_num_tokens(line) == 875318608908);
  }
}

std::uint64_t
min_num_tokens(std::ranges::range auto &&lines) {
  auto games{generate_games(lines)};
  return std::ranges::fold_left(games,
                                0ULL,
                                [](std::uint64_t prev, Game const &game) {
                                  return prev + min_num_tokens(game);
                                });
}
std::vector<Game>
generate_games(std::ranges::range auto &&lines) {
  std::vector<Game> games;
  std::size_t line_idx{};
  while (line_idx < lines.size()) {
    if (line_idx != 0) {
      ASSERT(lines[line_idx++] == "");
    }

    auto [step_x_A, step_y_A] =
        scn::scan<std::uint64_t, std::uint64_t>(lines[line_idx++],
                                                "Button A: X+{}, Y+{}")
            ->values();
    auto [step_x_B, step_y_B] =
        scn::scan<std::uint64_t, std::uint64_t>(lines[line_idx++],
                                                "Button B: X+{}, Y+{}")
            ->values();
    auto [prize_x, prize_y] =
        scn::scan<std::uint64_t, std::uint64_t>(lines[line_idx++],
                                                "Prize: X={}, Y={}")
            ->values();
    games.emplace_back(Button{step_x_A, step_y_A, 3},
                       Button{step_x_B, step_y_B, 1},
                       10'000'000'000'000ULL + prize_x,
                       10'000'000'000'000ULL + prize_y);
  }
  return games;
}

std::uint64_t
min_num_tokens(Game const &game) {
  std::uint64_t const &sax = game.button_A.step_x;
  std::uint64_t const &sbx = game.button_B.step_x;
  std::uint64_t const &say = game.button_A.step_y;
  std::uint64_t const &sby = game.button_B.step_y;
  std::uint64_t const &X = game.prize_x;
  std::uint64_t const &Y = game.prize_y;

  std::uint64_t const num_a = [=]() {
    return (sbx * Y > sby * X) ? (sbx * Y - sby * X) / (sbx * say - sby * sax)
                               : (sby * X - sbx * Y) / (sby * sax - sbx * say);
  }();
  std::uint64_t const num_b = (X - sax * num_a) / sbx;

  if (num_a * sax + num_b * sbx != X) {
    return 0;
  }
  if (num_a * say + num_b * sby != Y) {
    return 0;
  }
  return num_a * game.button_A.cost + num_b * game.button_B.cost;
}
} // namespace
