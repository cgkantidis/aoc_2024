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

namespace
{
void
tests();
std::uint64_t
total_cost_of_fence(std::ranges::range auto &&lines);
Matrix<char>
generate_grid(std::ranges::range auto &&lines);
std::uint64_t
visit_all_regions(Matrix<char> const &grid);
std::uint64_t
visit_region(Matrix<char> const &grid,
             Matrix<uint8_t> &visited_grid,
             std::size_t beg_row,
             std::size_t beg_col);
} // namespace

struct Plot
{
  std::size_t row;
  std::size_t col;
};

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
    ASSERT(total_cost_of_fence(lines) == 140);
  }
  {
    auto const lines = std::array{
        "OOOOO"sv,
        "OXOXO"sv,
        "OOOOO"sv,
        "OXOXO"sv,
        "OOOOO"sv,
    };
    ASSERT(total_cost_of_fence(lines) == 772);
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
    ASSERT(total_cost_of_fence(line) == 1930);
  }
}

std::uint64_t
total_cost_of_fence(std::ranges::range auto &&lines) {
  auto grid{generate_grid(lines)};
  return visit_all_regions(grid);
}

Matrix<char>
generate_grid(std::ranges::range auto &&lines) {
  Matrix<char> grid(lines.size(), lines[0].size());
  for (std::size_t row = 0; row < grid.rows(); ++row) {
    std::ranges::copy_n(lines[row].begin(),
                        static_cast<long>(grid.cols()),
                        grid.get_row(row));
  }
  return grid;
}

std::uint64_t
visit_all_regions(Matrix<char> const &grid) {
  std::uint64_t total_cost{};
  Matrix<uint8_t> visited_grid{grid.rows(), grid.cols(), 0};
  for (std::size_t row{}; row < visited_grid.rows(); ++row) {
    for (std::size_t col{}; col < visited_grid.cols(); ++col) {
      if (visited_grid(row, col)) {
        continue;
      }
      total_cost += visit_region(grid, visited_grid, row, col);
    }
  }
  return total_cost;
}

std::uint64_t
visit_region(Matrix<char> const &grid,
             Matrix<uint8_t> &visited_grid,
             std::size_t beg_row,
             std::size_t beg_col) {
  std::uint64_t area{};
  std::uint64_t perimeter{};

  std::unordered_set<Plot> to_visit;
  to_visit.emplace(beg_row, beg_col);

  while (!to_visit.empty()) {
    auto it = to_visit.begin();
    auto [row, col] = *it;
    to_visit.erase(it);

    visited_grid(row, col) = 1;
    ++area;

    // UP
    if (row == 0 || grid(row - 1, col) != grid(row, col)) {
      ++perimeter;
    } else if (!visited_grid(row - 1, col)) {
      to_visit.emplace(row - 1, col);
    }
    // LEFT
    if (col == 0 || grid(row, col - 1) != grid(row, col)) {
      ++perimeter;
    } else if (!visited_grid(row, col - 1)) {
      to_visit.emplace(row, col - 1);
    }
    // DOWN
    if (row == grid.rows() - 1 || grid(row + 1, col) != grid(row, col)) {
      ++perimeter;
    } else if (!visited_grid(row + 1, col)) {
      to_visit.emplace(row + 1, col);
    }
    // RIGHT
    if (col == grid.cols() - 1 || grid(row, col + 1) != grid(row, col)) {
      ++perimeter;
    } else if (!visited_grid(row, col + 1)) {
      to_visit.emplace(row, col + 1);
    }
  }

  return area * perimeter;
}

} // namespace
