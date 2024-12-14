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

struct Point
{
  std::size_t row;
  std::size_t col;

  bool
  operator<(Point const &other) const {
    return row < other.row || col < other.col;
  }
};

/// we need the operator==() to resolve hash collisions
bool
operator==(Point const &lhs, Point const &rhs) {
  return lhs.row == rhs.row && lhs.col == rhs.col;
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
struct std::hash<Point>
{
  std::size_t
  operator()(Point const &s) const noexcept {
    std::size_t h1 = std::hash<std::size_t>{}(s.row);
    std::size_t h2 = std::hash<std::size_t>{}(s.col);

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
sum_of_rating_of_trailheads(std::ranges::range auto &&lines);
Matrix<unsigned>
generate_grid(std::ranges::range auto &&lines);
std::uint64_t
get_trailhead_rating(Matrix<unsigned> const &grid,
                     std::size_t row,
                     std::size_t col);
[[maybe_unused]] void
print_grid(Matrix<unsigned> const &grid);
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

  fmt::println("{}", sum_of_rating_of_trailheads(lines));
  return 0;
}

namespace
{
void
tests() {
  using namespace std::literals::string_view_literals;
  {
    auto const lines = std::array{
        ".....0."sv,
        "..4321."sv,
        "..5..2."sv,
        "..6543."sv,
        "..7..4."sv,
        "..8765."sv,
        "..9...."sv,
    };
    ASSERT(sum_of_rating_of_trailheads(lines) == 3);
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
    ASSERT(sum_of_rating_of_trailheads(lines) == 13);
  }
  {
    auto const lines = std::array{
        "012345"sv,
        "123456"sv,
        "234567"sv,
        "345678"sv,
        "4.6789"sv,
        "56789."sv,
    };
    ASSERT(sum_of_rating_of_trailheads(lines) == 227);
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
    ASSERT(sum_of_rating_of_trailheads(lines) == 81);
  }
}

std::uint64_t
sum_of_rating_of_trailheads(std::ranges::range auto &&lines) {
  auto grid{generate_grid(lines)};
  std::uint64_t total_score{};
  for (std::size_t row = 0; row < grid.rows(); ++row) {
    for (std::size_t col = 0; col < grid.cols(); ++col) {
      if (grid(row, col) == 0) {
        // trailhead
        total_score += get_trailhead_rating(grid, row, col);
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
get_trailhead_rating(Matrix<unsigned> const &grid,
                     std::size_t head_row,
                     std::size_t head_col) {
  std::uint64_t rating{};

  std::vector<Point> trail;
  trail.emplace_back(head_row, head_col);

  while (!trail.empty()) {
    Point tail = trail.back();
    trail.pop_back();

    std::size_t row = tail.row;
    std::size_t col = tail.col;

    if (grid(row, col) == 9) {
      ++rating;
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

  return rating;
}

[[maybe_unused]] void
print_grid(Matrix<unsigned> const &grid) {
  for (std::size_t row = 0; row < grid.rows(); ++row) {
    for (std::size_t col = 0; col < grid.cols(); ++col) {
      if (grid(row, col) == UNINIT) {
        fmt::print(".");
      } else {
        fmt::print("{}", grid(row, col));
      }
    }
    fmt::println("");
  }
}
} // namespace
