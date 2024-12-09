#include "fmt/core.h"
#include "libassert/assert.hpp"
#include <cstdint>
#include <fstream>
#include <map>
#include <ranges>
#include <string>
#include <string_view>
#include <utility>

struct Location
{
  std::int64_t m_row;
  std::int64_t m_col;

  [[nodiscard]] bool
  is_in_grid(std::size_t num_rows, std::size_t num_cols) const {
    return m_row >= 0 && m_col >= 0 && static_cast<std::size_t>(m_row) < num_rows
           && static_cast<std::size_t>(m_col) < num_cols;
  }
};

class Antenna
{
private:
  Location m_loc;

public:
  Antenna(std::int64_t row, std::int64_t col) : m_loc{row, col} {}
  [[nodiscard]] std::pair<Location, Location>
  get_antinodes(Antenna const &other) const {
    std::int64_t row_diff{std::abs(m_loc.m_row - other.m_loc.m_row)};
    std::int64_t col_diff{std::abs(m_loc.m_col - other.m_loc.m_col)};
    if (m_loc.m_row <= other.m_loc.m_row) {
      if (m_loc.m_col <= other.m_loc.m_col) {
        // 1....
        // .A...
        // .....
        // ...a.
        // ....2
        return {{m_loc.m_row - row_diff, m_loc.m_col - col_diff},
                {other.m_loc.m_row + row_diff, other.m_loc.m_col + col_diff}};
      }
      // ....1
      // ...A.
      // .....
      // .a...
      // 2....
      return {{m_loc.m_row - row_diff, m_loc.m_col + col_diff},
              {other.m_loc.m_row + row_diff, other.m_loc.m_col - col_diff}};
    }
    if (m_loc.m_col <= other.m_loc.m_col) {
      // ....1
      // ...a.
      // .....
      // .A...
      // 2....
      return {{m_loc.m_row + row_diff, m_loc.m_col - col_diff},
              {other.m_loc.m_row - row_diff, other.m_loc.m_col + col_diff}};
    }
    // 2....
    // .a...
    // .....
    // ...A.
    // ....1
    return {{m_loc.m_row + row_diff, m_loc.m_col + col_diff},
            {other.m_loc.m_row - row_diff, other.m_loc.m_col - col_diff}};
  }
};

static void
tests();
static std::uint64_t
get_num_antinodes(std::ranges::range auto &&lines);
static std::map<char, std::vector<Antenna>>
parse_antenas(std::ranges::range auto &&lines,
              std::size_t num_rows,
              std::size_t num_cols);
static std::uint64_t
count_antinodes(std::vector<std::vector<bool>> const &antinode_grid);

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

  fmt::println("{}", get_num_antinodes(lines));
  return 0;
}

void
tests() {
  using namespace std::literals::string_view_literals;
  auto lines = std::vector{{"............"sv,
                            "........0..."sv,
                            ".....0......"sv,
                            ".......0...."sv,
                            "....0......."sv,
                            "......A....."sv,
                            "............"sv,
                            "............"sv,
                            "........A..."sv,
                            ".........A.."sv,
                            "............"sv,
                            "............"sv}};

  ASSERT(get_num_antinodes(lines) == 14);
}

std::uint64_t
get_num_antinodes(std::ranges::range auto &&lines) {
  std::size_t const num_rows = lines.size();
  std::size_t const num_cols = lines[0].size();
  auto const antenna_map = parse_antenas(lines, num_rows, num_cols);
  auto const antinode_grid =
      build_antinode_grid(antenna_map, num_rows, num_cols);

  return count_antinodes(antinode_grid);
}

std::map<char, std::vector<Antenna>>
parse_antenas(std::ranges::range auto &&lines,
              std::size_t num_rows,
              std::size_t num_cols) {
  std::map<char, std::vector<Antenna>> antennas;
  for (std::size_t row{0}; row < num_rows; ++row) {
    for (std::size_t col{0}; col < num_cols; ++col) {
      if (lines[row][col] == '.') {
        continue;
      }
      antennas[lines[row][col]].emplace_back(row, col);
    }
  }
  return antennas;
}

std::vector<std::vector<bool>>
build_antinode_grid(std::map<char, std::vector<Antenna>> const &antenna_map,
                    std::size_t num_rows,
                    std::size_t num_cols) {
  std::vector<std::vector<bool>> antinode_grid(num_rows,
                                               std::vector(num_cols, false));
  for (auto const &[key, antennas] : antenna_map) {
    std::size_t const num_antennas = antennas.size();
    for (std::size_t idx1{0}; idx1 < num_antennas - 1; ++idx1) {
      for (std::size_t idx2{idx1 + 1}; idx2 < num_antennas; ++idx2) {
        auto const [antinode1, antinode2] =
            antennas[idx1].get_antinodes(antennas[idx2]);
        if (antinode1.is_in_grid(num_rows, num_cols)) {
          antinode_grid[antinode1.m_row][antinode1.m_col] = true;
        }
        if (antinode2.is_in_grid(num_rows, num_cols)) {
          antinode_grid[antinode2.m_row][antinode2.m_col] = true;
        }
      }
    }
  }
  return antinode_grid;
}

std::uint64_t
count_antinodes(std::vector<std::vector<bool>> const &antinode_grid) {
  std::uint64_t num_antinodes{};
  for (auto const &line : antinode_grid) {
    for (auto const &col : line) {
      if (col) {
        ++num_antinodes;
      }
    }
  }
  return num_antinodes;
}
