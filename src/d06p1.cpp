#include "fmt/core.h"
#include "libassert/assert.hpp"
#include <algorithm>
#include <fstream>
#include <ranges>
#include <string>
#include <string_view>
#include <utility>

using namespace std::literals::string_view_literals;

enum class Direction
{
  UP,
  DOWN,
  LEFT,
  RIGHT
};

struct Guard
{
  std::size_t row;
  std::size_t col;
  Direction dir;
};

static Direction
char_to_dir(char ch) {
  switch (ch) {
  case '^':
    return Direction::UP;
  case '>':
    return Direction::RIGHT;
  case 'v':
    return Direction::DOWN;
  case '<':
    return Direction::LEFT;
  default:
    ASSERT(false, "", ch);
  }
  std::unreachable();
}

// static char
// dir_to_char(Direction dir) {
//   switch (dir) {
//   case Direction::UP:
//     return '^';
//   case Direction::RIGHT:
//     return '>';
//   case Direction::DOWN:
//     return 'v';
//   case Direction::LEFT:
//     return '<';
//   }
//   ASSERT(false, "", dir);
//   std::unreachable();
// }

static std::pair<std::vector<std::vector<char>>, Guard>
scan_grid(std::ranges::range auto &&lines) {
  std::size_t num_rows = lines.size();
  std::size_t num_cols = lines[0].size();
  Guard guard{0, 0, Direction::UP};

  std::vector<std::vector<char>> grid(num_rows,
                                      std::vector<char>(num_cols, '.'));
  for (std::size_t row = 0; row < num_rows; ++row) {
    for (std::size_t col = 0; col < num_cols; ++col) {
      grid[row][col] = lines[row][col];
      if (grid[row][col] != '.' && grid[row][col] != '#') {
        guard = Guard{row, col, char_to_dir(grid[row][col])};
        grid[row][col] = 'X';
      }
    }
  }

  return {grid, guard};
}

static bool
advance_guard(std::vector<std::vector<char>> &grid, Guard &guard) {
  switch (guard.dir) {
  case Direction::UP: {
    if (guard.row == 0) {
      return false; // exit the room
    }
    if (grid[guard.row - 1][guard.col] == '#') {
      guard.dir = Direction::RIGHT;
      return true;
    }
    --guard.row;
    break;
  }
  case Direction::RIGHT: {
    if (guard.col == grid[0].size() - 1) {
      return false; // exit the room
    }
    if (grid[guard.row][guard.col + 1] == '#') {
      guard.dir = Direction::DOWN;
      return true;
    }
    ++guard.col;
    break;
  }
  case Direction::DOWN: {
    if (guard.row + 1 == grid.size()) {
      return false; // exit the room
    }
    if (grid[guard.row + 1][guard.col] == '#') {
      guard.dir = Direction::LEFT;
      return true;
    }
    ++guard.row;
    break;
  }
  case Direction::LEFT: {
    if (guard.col == 0) {
      return false; // exit the room
    }
    if (grid[guard.row][guard.col - 1] == '#') {
      guard.dir = Direction::UP;
      return true;
    }
    --guard.col;
    break;
  }
  }
  // fmt::println("{} {} {}", guard.row, guard.col, dir_to_char(guard.dir));
  grid[guard.row][guard.col] = 'X';
  return true;
}

static std::uint64_t
count_xes(std::vector<std::vector<char>> const &grid) {
  return std::ranges::fold_left(
      grid,
      0ULL,
      [](std::uint64_t prev, std::vector<char> const &line) {
        return prev + static_cast<std::uint64_t>(std::ranges::count(line, 'X'));
      });
}

static std::uint64_t
count_visited_positions(std::ranges::range auto &&lines) {
  auto [grid, guard] = scan_grid(lines);
  while (advance_guard(grid, guard)) {}
  return count_xes(grid);
}

static void
tests() {
  auto lines = std::vector{{
      "....#....."sv,
      ".........#"sv,
      ".........."sv,
      "..#......."sv,
      ".......#.."sv,
      ".........."sv,
      ".#..^....."sv,
      "........#."sv,
      "#........."sv,
      "......#..."sv,
  }};

  ASSERT(count_visited_positions(lines) == 41);
}

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

  fmt::println("{}", count_visited_positions(lines));
  return 0;
}
