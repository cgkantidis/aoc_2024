#include <cstdint>
#include <fmt/core.h>
#include <fstream>
#include <libassert/assert.hpp>
#include <ranges>
#include <scn/scan.h>
#include <string>
#include <utility>

#include "matrix.hpp"

struct Robot
{
  std::uint64_t x;
  std::uint64_t y;
  std::int64_t vx;
  std::int64_t vy;
};

namespace
{
std::uint64_t
find_easter_egg(std::ranges::range auto &&lines,
                std::uint64_t seconds,
                std::uint64_t width,
                std::uint64_t length);
std::vector<Robot>
parse_robots(std::ranges::range auto &&lines);
void
advance_robots(std::vector<Robot> &robots,
               std::uint64_t seconds,
               std::uint64_t width,
               std::uint64_t length);
void
populate_grid(std::vector<Robot> const &robots, Matrix<char> &grid);
bool
is_tree(Matrix<char> const &grid);
} // namespace

int
main(int argc, char const *const *argv) {
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

  fmt::println("{}", find_easter_egg(lines, 1000000, 101, 103));
  return 0;
}

namespace
{
std::uint64_t
find_easter_egg(std::ranges::range auto &&lines,
                std::uint64_t seconds,
                std::uint64_t width,
                std::uint64_t length) {
  auto robots{parse_robots(lines)};
  Matrix<char> grid(length, width, ' ');
  for (std::uint64_t sec = 1; sec <= seconds; ++sec) {
    advance_robots(robots, 1, width, length);
    populate_grid(robots, grid);
    if (is_tree(grid)) {
      grid.print();
      return sec;
    }
    grid.clear(' ');
  }
  return 0;
}
std::vector<Robot>
parse_robots(std::ranges::range auto &&lines) {
  std::vector<Robot> robots;
  for (auto const &line : lines) {
    auto [x, y, vx, vy] =
        scn::scan<std::uint64_t, std::uint64_t, std::int64_t, std::int64_t>(
            line,
            "p={},{} v={},{}")
            ->values();
    robots.emplace_back(x, y, vx, vy);
  }
  return robots;
}

void
advance_robots(std::vector<Robot> &robots,
               std::uint64_t seconds,
               std::uint64_t width,
               std::uint64_t length) {
  for (auto &robot : robots) {
    robot.x = (robot.x + ((width + robot.vx) % width) * seconds) % width;
    robot.y = (robot.y + ((length + robot.vy) % length) * seconds) % length;
  }
}

void
populate_grid(std::vector<Robot> const &robots, Matrix<char> &grid) {
  for (Robot const &robot : robots) {
    grid(robot.y, robot.x) = 'X';
  }
}

bool
is_tree(Matrix<char> const &grid) {
  std::size_t num_touching_all{};
  for (std::size_t row = 1; row < grid.rows() - 1; ++row) {
    for (std::size_t col = 1; col < grid.cols() - 1; ++col) {
      auto middle = grid(row, col);
      if (middle != 'X') {
        continue;
      }
      for (std::size_t subrow = row - 1; subrow < row + 1; ++subrow) {
        for (std::size_t subcol = col - 1; subcol < col + 1; ++subcol) {
          if (grid(subrow, subcol) != middle) {
            goto nope;
          }
        }
      }
      ++num_touching_all;
    nope:
    }
  }
  return num_touching_all > 10;
}

} // namespace
