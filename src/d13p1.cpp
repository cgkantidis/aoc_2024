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
    ASSERT(min_num_tokens(lines) == 280);
  }
  {
    auto const lines = std::array{
        "Button A: X+26, Y+66"sv,
        "Button B: X+67, Y+21"sv,
        "Prize: X=12748, Y=12176"sv,
    };
    ASSERT(min_num_tokens(lines) == 0);
  }
  {
    auto const line = std::array{
        "Button A: X+17, Y+86"sv,
        "Button B: X+84, Y+37"sv,
        "Prize: X=7870, Y=6450"sv,
    };
    ASSERT(min_num_tokens(line) == 200);
  }
  {
    auto const line = std::array{
        "Button A: X+69, Y+23"sv,
        "Button B: X+27, Y+71"sv,
        "Prize: X=18641, Y=10279"sv,
    };
    ASSERT(min_num_tokens(line) == 0);
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
    ASSERT(min_num_tokens(line) == 480);
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
                       prize_x,
                       prize_y);
  }
  return games;
}

std::uint64_t
min_num_tokens(Game const &game) {
  std::unordered_map<Location, std::uint64_t> position_cost;
  position_cost.emplace(std::piecewise_construct,
                        std::forward_as_tuple(game.prize_y, game.prize_x),
                        std::forward_as_tuple(0));

  std::queue<Location> positions;
  positions.emplace(game.prize_y, game.prize_x);
  while (!positions.empty()) {
    auto top = positions.front();
    positions.pop();

    auto top_it = position_cost.find(top);

    if (top.col >= game.button_A.step_x && top.row >= game.button_A.step_y) {
      Location next_pos{top.row - game.button_A.step_y,
                        top.col - game.button_A.step_x};
      auto next_it = position_cost.find(next_pos);
      auto next_cost = top_it->second + game.button_A.cost;
      if (next_it == position_cost.end()) {
        positions.emplace(next_pos);
        position_cost.emplace(next_pos, next_cost);
      } else if (next_it->second > next_cost) {
        positions.emplace(next_pos);
        next_it->second = next_cost;
      }
    }
    if (top.col >= game.button_B.step_x && top.row >= game.button_B.step_y) {
      Location next_pos{top.row - game.button_B.step_y,
                        top.col - game.button_B.step_x};
      auto next_it = position_cost.find(next_pos);
      auto next_cost = top_it->second + game.button_B.cost;
      if (next_it == position_cost.end()) {
        positions.emplace(next_pos);
        position_cost.emplace(next_pos, next_cost);
      } else if (next_it->second > next_cost) {
        positions.emplace(next_pos);
        next_it->second = next_cost;
      }
    }
  }

  auto find_it = position_cost.find({0, 0});
  if (find_it != position_cost.end()) {
    return find_it->second;
  }

  return 0;
}
} // namespace
