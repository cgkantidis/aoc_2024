#include <array>
#include <cstdint>
#include <fmt/core.h>
#include <fstream>
#include <functional> // multiplies
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
  std::int64_t vx;
  std::int64_t vy;
};

namespace
{
void
tests();
std::uint64_t
compute_safety_factor(std::ranges::range auto &&lines,
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
std::uint64_t
compute_safety_factor(std::vector<Robot> const &robots,
                      std::uint64_t width,
                      std::uint64_t length);
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

  fmt::println("{}", compute_safety_factor(lines, 100, 101, 103));
  return 0;
}

namespace
{
void
tests() {
  using namespace std::literals::string_view_literals;
  {
    auto const lines = std::array{"p=2,4 v=2,-3"sv};
    ASSERT(compute_safety_factor(lines, 5, 11, 7) == 0);
  }
  {
    auto const lines = std::array{
        "p=0,4 v=3,-3"sv,
        "p=6,3 v=-1,-3"sv,
        "p=10,3 v=-1,2"sv,
        "p=2,0 v=2,-1"sv,
        "p=0,0 v=1,3"sv,
        "p=3,0 v=-2,-2"sv,
        "p=7,6 v=-1,-3"sv,
        "p=3,0 v=-1,-2"sv,
        "p=9,3 v=2,3"sv,
        "p=7,3 v=-1,2"sv,
        "p=2,4 v=2,-3"sv,
        "p=9,5 v=-3,-3"sv,
    };
    ASSERT(compute_safety_factor(lines, 100, 11, 7) == 12);
  }
}

std::uint64_t
compute_safety_factor(std::ranges::range auto &&lines,
                      std::uint64_t seconds,
                      std::uint64_t width,
                      std::uint64_t length) {
  auto robots{parse_robots(lines)};
  advance_robots(robots, seconds, width, length);
  return compute_safety_factor(robots, width, length);
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

std::uint64_t
compute_safety_factor(std::vector<Robot> const &robots,
                      std::uint64_t width,
                      std::uint64_t length) {
  std::array<std::uint64_t, 4> quadrant_count{};
  for (Robot const &robot : robots) {
    if (robot.x < width / 2 && robot.y < length / 2) {
      ++quadrant_count[0];
    } else if (robot.x > width / 2 && robot.y < length / 2) {
      ++quadrant_count[1];
    } else if (robot.x < width / 2 && robot.y > length / 2) {
      ++quadrant_count[2];
    } else if (robot.x > width / 2 && robot.y > length / 2) {
      ++quadrant_count[3];
    }
  }
  return std::ranges::fold_left(quadrant_count,
                                1,
                                std::multiplies<std::uint64_t>{});
}
} // namespace
