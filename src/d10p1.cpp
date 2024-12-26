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

static constexpr auto UNINIT = std::numeric_limits<unsigned>::max();

enum Dir : std::uint8_t
{
  UP,
  LEFT,
  DOWN,
  RIGHT
};

static constexpr std::initializer_list<Dir> ALL_DIRS{Dir::UP,
                                                     Dir::LEFT,
                                                     Dir::DOWN,
                                                     Dir::RIGHT};

namespace
{
void
tests();
std::uint64_t
sum_of_score_of_trailheads(std::ranges::range auto &&lines);
Matrix<unsigned>
generate_grid(std::ranges::range auto &&lines);
std::uint64_t
get_trailhead_score(Matrix<unsigned> const &grid,
                    std::size_t row,
                    std::size_t col);
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

  fmt::println("{}", sum_of_score_of_trailheads(lines));
  return 0;
}

namespace
{
void
tests() {
  using namespace std::literals::string_view_literals;
  {
    auto const lines = std::array{
        "0123"sv,
        "1234"sv,
        "8765"sv,
        "9876"sv,
    };
    ASSERT(sum_of_score_of_trailheads(lines) == 1);
  }
  {
    auto const lines = std::array{
        "...0..."sv,
        "...1..."sv,
        "...2..."sv,
        "6543456"sv,
        "7.....7"sv,
        "8.....8"sv,
        "9.....9"sv,
    };
    ASSERT(sum_of_score_of_trailheads(lines) == 2);
  }
  {
    auto const lines = std::array{
        "..90..9"sv,
        "...1.98"sv,
        "...2..7"sv,
        "6543456"sv,
        "765.987"sv,
        "876...."sv,
        "987...."sv,
    };
    ASSERT(sum_of_score_of_trailheads(lines) == 4);
  }
  {
    auto const lines = std::array{
        "10..9.."sv,
        "2...8.."sv,
        "3...7.."sv,
        "4567654"sv,
        "...8..3"sv,
        "...9..2"sv,
        ".....01"sv,
    };
    ASSERT(sum_of_score_of_trailheads(lines) == 3);
  }
  {
    auto const lines = std::array{
        "89010123"sv,
        "78121874"sv,
        "87430965"sv,
        "96549874"sv,
        "45678903"sv,
        "32019012"sv,
        "01329801"sv,
        "10456732"sv,
    };
    ASSERT(sum_of_score_of_trailheads(lines) == 36);
  }
}

std::uint64_t
sum_of_score_of_trailheads(std::ranges::range auto &&lines) {
  auto grid{generate_grid(lines)};
  std::uint64_t total_score{};
  for (std::size_t row = 0; row < grid.rows(); ++row) {
    for (std::size_t col = 0; col < grid.cols(); ++col) {
      if (grid(row, col) == 0) {
        // trailhead
        total_score += get_trailhead_score(grid, row, col);
      }
    }
  }
  return total_score;
}

Matrix<unsigned>
generate_grid(std::ranges::range auto &&lines) {
  Matrix<unsigned> grid(lines.size(), lines[0].size());
  for (std::size_t line_idx = 0; line_idx < grid.rows(); ++line_idx) {
    std::ranges::transform(lines[line_idx], grid.get_row(line_idx), [](char ch) {
      return ch == '.' ? UNINIT : static_cast<unsigned>(ch - '0');
    });
  }
  return grid;
}

std::uint64_t
get_trailhead_score(Matrix<unsigned> const &grid,
                    std::size_t head_row,
                    std::size_t head_col) {
  std::unordered_set<Location> tails;

  std::vector<Location> trail;
  trail.emplace_back(head_row, head_col);

  while (!trail.empty()) {
    Location tail = trail.back();
    trail.pop_back();

    std::size_t row = tail.row;
    std::size_t col = tail.col;

    if (grid(row, col) == 9) {
      tails.emplace(row, col);
      continue;
    }

    for (Dir dir : ALL_DIRS) {
      switch (dir) {
      case Dir::UP: {
        if (row != 0 && grid(row - 1, col) == grid(row, col) + 1) {
          trail.emplace_back(row - 1, col);
        }
        break;
      }
      case Dir::LEFT: {
        if (col != 0 && grid(row, col - 1) == grid(row, col) + 1) {
          trail.emplace_back(row, col - 1);
        }
        break;
      }
      case Dir::DOWN: {
        if (row != grid.rows() - 1 && grid(row + 1, col) == grid(row, col) + 1) {
          trail.emplace_back(row + 1, col);
        }
        break;
      }
      case Dir::RIGHT: {
        if (col != grid.cols() - 1 && grid(row, col + 1) == grid(row, col) + 1) {
          trail.emplace_back(row, col + 1);
        }
        break;
      }
      }
    }
  }

  return tails.size();
}

} // namespace
