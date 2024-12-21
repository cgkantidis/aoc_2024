#include <array>
#include <cstdint>
#include <fmt/core.h>
#include <fstream>
#include <libassert/assert.hpp>
#include <ranges>
#include <scn/scan.h>
#include <string>
#include <string_view>
#include <utility>

#include "matrix.hpp"

struct Robot
{
  std::uint64_t x;
  std::uint64_t y;
};

enum class Move
{
  UP,
  DOWN,
  LEFT,
  RIGHT,
};

namespace
{
void
tests();
std::uint64_t
sum_of_gps_coord(std::ranges::range auto &&lines);
std::tuple<Robot, std::vector<Move>, Matrix<char>>
parse_robot_moves_grid(std::ranges::range auto &&lines);
void
perform_movements(Robot &robot,
                  std::vector<Move> const &moves,
                  Matrix<char> &grid);
void
move_up(Robot &robot, Matrix<char> &grid);
void
move_down(Robot &robot, Matrix<char> &grid);
void
move_left(Robot &robot, Matrix<char> &grid);
void
move_right(Robot &robot, Matrix<char> &grid);
std::uint64_t
sum_of_gps_coord(Matrix<char> const &grid);
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

  fmt::println("{}", sum_of_gps_coord(lines));
  return 0;
}

namespace
{
void
tests() {
  using namespace std::literals::string_view_literals;
  {
    auto const lines = std::array{
        "########"sv,
        "#..O.O.#"sv,
        "##@.O..#"sv,
        "#...O..#"sv,
        "#.#.O..#"sv,
        "#...O..#"sv,
        "#......#"sv,
        "########"sv,
        ""sv,
        "<^^>>>vv<v>>v<<"sv,
    };
    ASSERT(sum_of_gps_coord(lines) == 2028);
  }
  {
    auto const lines = std::array{
        "##########"sv,
        "#..O..O.O#"sv,
        "#......O.#"sv,
        "#.OO..O.O#"sv,
        "#..O@..O.#"sv,
        "#O#..O...#"sv,
        "#O..O..O.#"sv,
        "#.OO.O.OO#"sv,
        "#....O...#"sv,
        "##########"sv,
        ""sv,
        "<vv>^<v^>v>^vv^v>v<>v^v<v<^vv<<<^><<><>>v<vvv<>^v^>^<<<><<v<<<v^vv^v>^"sv,
        "vvv<<^>^v^^><<>>><>^<<><^vv^^<>vvv<>><^^v>^>vv<>v<<<<v<^v>^<^^>>>^<v<v"sv,
        "><>vv>v^v^<>><>>>><^^>vv>v<^^^>>v^v^<^^>v^^>v^<^v>v<>>v^v^<v>v^^<^^vv<"sv,
        "<<v<^>>^^^^>>>v^<>vvv^><v<<<>^^^vv^<vvv>^>v<^^^^v<>^>vvvv><>>v^<<^^^^^"sv,
        "^><^><>>><>^^<<^^v>>><^<v>^<vv>>v>>>^v><>^v><<<<v>>v<v<v>vvv>^<><<>^><"sv,
        "^>><>^v<><^vvv<^^<><v<<<<<><^v<<<><<<^^<v<^^^><^>>^<v^><<<^>>^v<v^v<v^"sv,
        ">^>>^v>vv>^<<^v<>><<><<v<<v><>v<^vv<<<>^^v^>^^>>><<^v>>v^v><^^>>^<>vv^"sv,
        "<><^^>^^^<><vvvvv^v<v<<>^v<v>v<<^><<><<><<<^^<<<^<<>><<><^^^>^^<>^>v<>"sv,
        "^^>vv<^v^v<vv>^<><v<^v>^^^>>>^^vvv^>vvv<>>>^<^>>>>>^<<^v>^vvv<>^<><<v>"sv,
        "v^^>>><<^^<>>^v^<v^vv<>v^<<>^<^v^v><^<<<><<^<v><v<>vv>>v><v^<vv<>v^<<^"sv,
    };
    ASSERT(sum_of_gps_coord(lines) == 10092);
  }
}

std::uint64_t
sum_of_gps_coord(std::ranges::range auto &&lines) {
  auto [robot, moves, grid] = parse_robot_moves_grid(lines);
  perform_movements(robot, moves, grid);
  return sum_of_gps_coord(grid);
}

std::tuple<Robot, std::vector<Move>, Matrix<char>>
parse_robot_moves_grid(std::ranges::range auto &&lines) {
  // find the number of rows in the grid
  auto const rows = static_cast<std::size_t>(
      std::distance(lines.begin(), std::ranges::find(lines, "")));
  auto const cols = lines[0].size();

  Matrix<char> grid(rows, cols, ' ');
  Robot robot{};
  for (std::size_t row{}; row < rows; ++row) {
    for (std::size_t col{}; col < cols; ++col) {
      auto ch = lines[row][col];
      switch (ch) {
      case '#':
      case 'O': {
        grid(row, col) = ch;
        break;
      }
      case '@': {
        robot = Robot{row, col};
      }
      }
    }
  }

  std::vector<Move> moves;
  for (auto const &line : lines | std::views::drop(rows + 1)) {
    std::ranges::transform(line, std::back_inserter(moves), [](char ch) {
      switch (ch) {
      case '^': {
        return Move::UP;
      }
      case 'v': {
        return Move::DOWN;
      }
      case '<': {
        return Move::LEFT;
      }
      case '>': {
        return Move::RIGHT;
      }
      default: {
        UNREACHABLE(ch);
      }
      }
    });
  }
  return {robot, moves, grid};
}

void
perform_movements(Robot &robot,
                  std::vector<Move> const &moves,
                  Matrix<char> &grid) {
  for (Move const &move : moves) {
    switch (move) {
    case Move::UP: {
      move_up(robot, grid);
      break;
    }
    case Move::DOWN: {
      move_down(robot, grid);
      break;
    }
    case Move::LEFT: {
      move_left(robot, grid);
      break;
    }
    case Move::RIGHT: {
      move_right(robot, grid);
      break;
    }
    }
  }
}

void
move_up(Robot &robot, Matrix<char> &grid) {
  // search for a space moving up
  // if we find a block, no movement is done
  auto row = robot.y - 1;
  auto col = robot.x;
  while (grid(row, col) == 'O') {
    --row;
  }
  // if we found a block, no movement can be done
  if (grid(row, col) == '#') {
    return;
  }
  // we found a space
  ASSERT(grid(row, col) == ' ');
  while (row != robot.y - 1) {
    grid(row, col) = 'O';
    ++row;
  }
  grid(row, col) = ' ';
  robot.y = row;
}
void
move_down(Robot &robot, Matrix<char> &grid) {
  // search for a space moving down
  // if we find a block, no movement is done
  auto row = robot.y + 1;
  auto col = robot.x;
  while (grid(row, col) == 'O') {
    ++row;
  }
  // if we found a block, no movement can be done
  if (grid(row, col) == '#') {
    return;
  }
  // we found a space
  ASSERT(grid(row, col) == ' ');
  while (row != robot.y + 1) {
    grid(row, col) = 'O';
    --row;
  }
  grid(row, col) = ' ';
  robot.y = row;
}
void
move_left(Robot &robot, Matrix<char> &grid) {
  // search for a space moving left
  // if we find a block, no movement is done
  auto row = robot.y;
  auto col = robot.x - 1;
  while (grid(row, col) == 'O') {
    --col;
  }
  // if we found a block, no movement can be done
  if (grid(row, col) == '#') {
    return;
  }
  // we found a space
  ASSERT(grid(row, col) == ' ');
  while (col != robot.x - 1) {
    grid(row, col) = 'O';
    ++col;
  }
  grid(row, col) = ' ';
  robot.x = col;
}
void
move_right(Robot &robot, Matrix<char> &grid) {
  // search for a space moving right
  // if we find a block, no movement is done
  auto row = robot.y;
  auto col = robot.x + 1;
  while (grid(row, col) == 'O') {
    ++col;
  }
  // if we found a block, no movement can be done
  if (grid(row, col) == '#') {
    return;
  }
  // we found a space
  ASSERT(grid(row, col) == ' ');
  while (col != robot.x + 1) {
    grid(row, col) = 'O';
    --col;
  }
  grid(row, col) = ' ';
  robot.x = col;
}

std::uint64_t
sum_of_gps_coord(Matrix<char> const &grid) {
  std::uint64_t total{};
  for (std::size_t row{}; row < grid.rows(); ++row) {
    for (std::size_t col{}; col < grid.cols(); ++col) {
      if (grid(row, col) == 'O') {
        total += 100 * row + col;
      }
    }
  }
  return total;
}
} // namespace
