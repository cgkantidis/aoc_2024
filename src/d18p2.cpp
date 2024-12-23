#include <array>
#include <cstdint>
#include <deque> // deque
#include <fmt/core.h>
#include <fstream>
#include <libassert/assert.hpp>
#include <ranges>
#include <scn/scan.h>
#include <string>
#include <string_view>
#include <unordered_map> // unordered_map
#include <utility>

#include "matrix.hpp"
#include "utility.hpp"

struct Location
{
  std::size_t row;
  std::size_t col;
};

/// we need the operator==() to resolve hash collisions
bool
operator==(Location const &lhs, Location const &rhs) {
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
struct std::hash<Location>
{
  std::size_t
  operator()(Location const &loc) const noexcept {
    std::size_t h1 = std::hash<std::size_t>{}(loc.row);
    std::size_t h2 = std::hash<std::size_t>{}(loc.col);

    std::size_t ret_val = 0;
    hash_combine(ret_val, h1, h2);
    return ret_val;
  }
};

namespace
{
void
tests();
std::string
byte_blocking_path(std::ranges::range auto &&lines,
                   std::uint64_t rows,
                   std::uint64_t cols);
Matrix<char>
build_grid(std::uint64_t rows, std::uint64_t cols);
std::string
byte_blocking_path(std::ranges::range auto &&lines, Matrix<char> &grid);
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

  fmt::println("{}", byte_blocking_path(lines, 71, 71));
  return 0;
}

namespace
{
void
tests() {
  using namespace std::literals::string_view_literals;
  {
    auto const lines = std::array{
        "5,4"sv, "4,2"sv, "4,5"sv, "3,0"sv, "2,1"sv, "6,3"sv, "2,4"sv,
        "1,5"sv, "0,6"sv, "3,3"sv, "2,6"sv, "5,1"sv, "1,2"sv, "5,5"sv,
        "2,5"sv, "6,5"sv, "1,4"sv, "0,4"sv, "6,4"sv, "1,1"sv, "6,1"sv,
        "1,0"sv, "0,5"sv, "1,6"sv, "2,0"sv,
    };
    ASSERT(byte_blocking_path(lines, 7, 7) == "6,1");
  }
}

std::string
byte_blocking_path(std::ranges::range auto &&lines,
                   std::uint64_t rows,
                   std::uint64_t cols) {
  auto grid = build_grid(rows, cols);
  return byte_blocking_path(lines, grid);
}

Matrix<char>
build_grid(std::uint64_t rows, std::uint64_t cols) {
  Matrix<char> grid(rows + 2, cols + 2, ' ');
  for (std::size_t row = 0; row < rows + 2; ++row) {
    grid(row, 0) = '#';
    grid(row, cols + 1) = '#';
  }
  for (std::size_t col = 0; col < rows + 2; ++col) {
    grid(0, col) = '#';
    grid(rows + 1, col) = '#';
  }
  return grid;
}

std::string
byte_blocking_path(std::ranges::range auto &&lines, Matrix<char> &grid) {
  Location finish{grid.rows() - 2, grid.cols() - 2};
  for (auto const &[line_idx, line] : std::views::enumerate(lines)) {
    auto col_row =
        split(line, ",") | std::views::transform(str_to_int<std::size_t>);
    grid(col_row[1] + 1, col_row[0] + 1) = '#';

    std::unordered_map<Location, std::uint64_t> cost_map;
    cost_map[{1, 1}] = 0;

    std::deque<Location> to_visit;
    to_visit.emplace_back(1, 1);

    while (!to_visit.empty()) {
      auto cur = to_visit.front();
      auto next_cost = cost_map[cur] + 1;

      to_visit.pop_front();
      auto [row, col] = cur;
      for (Location next_loc : {Location{row - 1, col},
                                Location{row + 1, col},
                                Location{row, col - 1},
                                Location{row, col + 1}}) {
        if (grid(next_loc.row, next_loc.col) != '#') {
          auto find_it = cost_map.find(next_loc);
          if (find_it == cost_map.end()) {
            cost_map.emplace_hint(find_it, next_loc, next_cost);
            to_visit.emplace_back(next_loc);
          } else if (find_it->second > next_cost) {
            find_it->second = next_cost;
            to_visit.emplace_back(next_loc);
          }
        }
      }
    }

    auto find_it = cost_map.find(finish);
    if (find_it == cost_map.end()) {
      return fmt::format("{},{}", col_row[0], col_row[1]);
    }

    if (line_idx % 10 != 0) {
      continue;
    }

    auto cost = find_it->second;
    Location loc = finish;
    while (true) {
      grid(loc.row, loc.col) = 'O';
      if (loc.row == 1 && loc.col == 1) {
        break;
      }

      --cost;

      auto up_it = cost_map.find({loc.row - 1, loc.col});
      if (up_it != cost_map.end() && up_it->second == cost) {
        loc = up_it->first;
        continue;
      }
      auto down_it = cost_map.find({loc.row + 1, loc.col});
      if (down_it != cost_map.end() && down_it->second == cost) {
        loc = down_it->first;
        continue;
      }
      auto left_it = cost_map.find({loc.row, loc.col - 1});
      if (left_it != cost_map.end() && left_it->second == cost) {
        loc = left_it->first;
        continue;
      }
      auto right_it = cost_map.find({loc.row, loc.col + 1});
      if (right_it != cost_map.end() && right_it->second == cost) {
        loc = right_it->first;
        continue;
      }
    }
    grid.print();
    for (std::size_t row{}; row < grid.rows(); ++row) {
      for (std::size_t col{}; col < grid.cols(); ++col) {
        if (grid(row, col) == 'O') {
          grid(row, col) = ' ';
        }
      }
    }
  }
  UNREACHABLE();
}
} // namespace
