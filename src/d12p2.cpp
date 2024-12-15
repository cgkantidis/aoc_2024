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

struct Plot
{
  std::size_t row;
  std::size_t col;
};

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

/// we need the operator==() to resolve hash collisions
bool
operator==(Plot const &lhs, Plot const &rhs) {
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
struct std::hash<Plot>
{
  std::size_t
  operator()(Plot const &s) const noexcept {
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
total_cost_of_fence(std::ranges::range auto &&lines);
Matrix<char>
generate_augmented_grid(std::ranges::range auto &&lines);
std::uint64_t
visit_all_regions(Matrix<char> const &grid);
std::pair<std::uint64_t, std::uint64_t>
region_area_and_perimenter(Matrix<char> const &grid,
                           Matrix<bool> &area_visited,
                           Plot beg_plot);
bool
top_left_outer(Matrix<char> const &grid, Plot plot);
bool
bottom_left_outer(Matrix<char> const &grid, Plot plot);
bool
top_right_outer(Matrix<char> const &grid, Plot plot);
bool
bottom_right_outer(Matrix<char> const &grid, Plot plot);
bool
top_left_inner(Matrix<char> const &grid, Plot plot);
bool
bottom_left_inner(Matrix<char> const &grid, Plot plot);
bool
top_right_inner(Matrix<char> const &grid, Plot plot);
bool
bottom_right_inner(Matrix<char> const &grid, Plot plot);
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

  fmt::println("{}", total_cost_of_fence(lines));
  return 0;
}

namespace
{
void
tests() {
  using namespace std::literals::string_view_literals;
  {
    auto const lines = std::array{
        "AAAA"sv,
        "BBCD"sv,
        "BBCC"sv,
        "EEEC"sv,
    };
    ASSERT(total_cost_of_fence(lines) == 80);
  }
  {
    auto const lines = std::array{
        "EEEEE"sv,
        "EXXXX"sv,
        "EEEEE"sv,
        "EXXXX"sv,
        "EEEEE"sv,
    };
    ASSERT(total_cost_of_fence(lines) == 236);
  }
  {
    auto const line = std::array{
        "AAAAAA"sv,
        "AAABBA"sv,
        "AAABBA"sv,
        "ABBAAA"sv,
        "ABBAAA"sv,
        "AAAAAA"sv,
    };
    ASSERT(total_cost_of_fence(line) == 368);
  }
  {
    auto const line = std::array{
        "RRRRIICCFF"sv,
        "RRRRIICCCF"sv,
        "VVRRRCCFFF"sv,
        "VVRCCCJFFF"sv,
        "VVVVCJJCFE"sv,
        "VVIVCCJJEE"sv,
        "VVIIICJJEE"sv,
        "MIIIIIJJEE"sv,
        "MIIISIJEEE"sv,
        "MMMISSJEEE"sv,
    };
    ASSERT(total_cost_of_fence(line) == 1206);
  }
}

std::uint64_t
total_cost_of_fence(std::ranges::range auto &&lines) {
  auto grid{generate_augmented_grid(lines)};
  return visit_all_regions(grid);
}

Matrix<char>
generate_augmented_grid(std::ranges::range auto &&lines) {
  auto num_rows = lines.size();
  auto num_cols = lines[0].size();
  // the grid will have a perimeter of '.', so that we don't have to check that
  // we are inside the limits
  Matrix<char> grid(num_rows + 2, num_cols + 2, '.');
  for (std::size_t row = 0; row < num_rows; ++row) {
    for (std::size_t col = 0; col < num_cols; ++col) {
      grid(row + 1, col + 1) = lines[row][col];
    }
  }
  return grid;
}

std::uint64_t
visit_all_regions(Matrix<char> const &grid) {
  std::uint64_t total_cost{};
  Matrix<bool> area_visited{grid.rows(), grid.cols(), false};
  for (std::size_t row{1}; row < area_visited.rows() - 1; ++row) {
    for (std::size_t col{1}; col < area_visited.cols() - 1; ++col) {
      if (area_visited(row, col)) {
        continue;
      }
      auto [area, perimeter] =
          region_area_and_perimenter(grid, area_visited, Plot{row, col});
      total_cost += area * perimeter;
    }
  }
  return total_cost;
}

std::pair<std::uint64_t, std::uint64_t>
region_area_and_perimenter(Matrix<char> const &grid,
                           Matrix<bool> &area_visited,
                           Plot beg_plot) {
  std::uint64_t area{};
  std::uint64_t perimeter{};

  std::unordered_set<Plot> to_visit;
  to_visit.emplace(beg_plot);

  while (!to_visit.empty()) {
    auto it = to_visit.begin();
    auto plot = *it;
    auto [row, col] = plot;
    to_visit.erase(it);

    area_visited(row, col) = true;
    ++area;

    if (top_left_outer(grid, plot)) {
      ++perimeter;
    }
    if (top_right_outer(grid, plot)) {
      ++perimeter;
    }
    if (bottom_left_outer(grid, plot)) {
      ++perimeter;
    }
    if (bottom_right_outer(grid, plot)) {
      ++perimeter;
    }
    if (top_left_inner(grid, plot)) {
      ++perimeter;
    }
    if (top_right_inner(grid, plot)) {
      ++perimeter;
    }
    if (bottom_left_inner(grid, plot)) {
      ++perimeter;
    }
    if (bottom_right_inner(grid, plot)) {
      ++perimeter;
    }

    // UP
    if (row != 0 && grid(row - 1, col) == grid(row, col)
        && !area_visited(row - 1, col)) {
      to_visit.emplace(row - 1, col);
    }
    // LEFT
    if (col != 0 && grid(row, col - 1) == grid(row, col)
        && !area_visited(row, col - 1)) {
      to_visit.emplace(row, col - 1);
    }
    // DOWN
    if (row != grid.rows() - 1 && grid(row + 1, col) == grid(row, col)
        && !area_visited(row + 1, col)) {
      to_visit.emplace(row + 1, col);
    }
    // RIGHT
    if (col != grid.cols() - 1 && grid(row, col + 1) == grid(row, col)
        && !area_visited(row, col + 1)) {
      to_visit.emplace(row, col + 1);
    }
  }

  return {area, perimeter};
}

bool
top_left_outer(Matrix<char> const &grid, Plot plot) {
  auto [row, col] = plot;
  auto p = grid(row, col);
  return grid(row, col - 1) != p && grid(row - 1, col) != p;
}

bool
bottom_left_outer(Matrix<char> const &grid, Plot plot) {
  auto [row, col] = plot;
  auto p = grid(row, col);
  return grid(row, col - 1) != p && grid(row + 1, col) != p;
}

bool
top_right_outer(Matrix<char> const &grid, Plot plot) {
  auto [row, col] = plot;
  auto p = grid(row, col);
  return grid(row, col + 1) != p && grid(row - 1, col) != p;
}

bool
bottom_right_outer(Matrix<char> const &grid, Plot plot) {
  auto [row, col] = plot;
  auto p = grid(row, col);
  return grid(row, col + 1) != p && grid(row + 1, col) != p;
}

bool
top_left_inner(Matrix<char> const &grid, Plot plot) {
  auto [row, col] = plot;
  auto p = grid(row, col);
  return grid(row, col + 1) == p && grid(row + 1, col) == p
         && grid(row + 1, col + 1) != p;
}

bool
bottom_left_inner(Matrix<char> const &grid, Plot plot) {
  auto [row, col] = plot;
  auto p = grid(row, col);
  return grid(row, col + 1) == p && grid(row - 1, col) == p
         && grid(row - 1, col + 1) != p;
}

bool
top_right_inner(Matrix<char> const &grid, Plot plot) {
  auto [row, col] = plot;
  auto p = grid(row, col);
  return grid(row, col - 1) == p && grid(row + 1, col) == p
         && grid(row + 1, col - 1) != p;
}

bool
bottom_right_inner(Matrix<char> const &grid, Plot plot) {
  auto [row, col] = plot;
  auto p = grid(row, col);
  return grid(row, col - 1) == p && grid(row - 1, col) == p
         && grid(row - 1, col - 1) != p;
}
} // namespace
