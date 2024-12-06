#include "fmt/core.h"
#include "libassert/assert.hpp"
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
advance_guard(std::vector<std::vector<char>> &grid,
              Guard &guard,
              bool &in_loop,
              unsigned &num_turns) {
  if (num_turns >= 4) {
    in_loop = true;
    return false;
  }

  switch (guard.dir) {
  case Direction::UP: {
    if (guard.row == 0) {
      return false; // exit the room
    }
    if (grid[guard.row - 1][guard.col] == '#') {
      guard.dir = Direction::RIGHT;
      ++num_turns;
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
      ++num_turns;
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
      ++num_turns;
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
      ++num_turns;
      return true;
    }
    --guard.col;
    break;
  }
  }
  if (grid[guard.row][guard.col] != 'X') {
    num_turns = 0;
  }
  // fmt::println("{} {} {}", guard.row, guard.col, dir_to_char(guard.dir));
  grid[guard.row][guard.col] = 'X';
  return true;
}

static std::uint64_t
count_possible_loops(std::ranges::range auto &&lines) {
  auto [grid, guard] = scan_grid(lines);
  auto num_rows = grid.size();
  auto num_cols = grid[0].size();
  std::uint64_t num_loops{};
  for (std::size_t row = 0; row < num_rows; ++row) {
    for (std::size_t col = 0; col < num_cols; ++col) {
      if (grid[row][col] != '.') {
        continue;
      }

      auto grid_cpy{grid};
      auto guard_cpy{guard};
      grid_cpy[row][col] = '#';
      bool in_loop{false};
      unsigned num_turns{}; // number of turns without finding empty space
      while (advance_guard(grid_cpy, guard_cpy, in_loop, num_turns)) {}
      if (in_loop) {
        ++num_loops;
      }
    }
  }
  return num_loops;
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

  ASSERT(count_possible_loops(lines) == 6);
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

  fmt::println("{}", count_possible_loops(lines));
  return 0;
}

