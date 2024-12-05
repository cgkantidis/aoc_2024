#include <fmt/core.h>
#include <fstream>
#include <ranges>
#include <string>
#include <string_view>

#include <libassert/assert.hpp>

using namespace std::literals::string_view_literals;

bool
scan_right(std::ranges::range auto &&lines, std::size_t row, std::size_t col) {
  return col + 3 < lines[row].size() && lines[row][col] == 'X'
         && lines[row][col + 1] == 'M' && lines[row][col + 2] == 'A'
         && lines[row][col + 3] == 'S';
}

bool
scan_left(std::ranges::range auto &&lines, std::size_t row, std::size_t col) {
  return col >= 3 && lines[row][col] == 'X' && lines[row][col - 1] == 'M'
         && lines[row][col - 2] == 'A' && lines[row][col - 3] == 'S';
}

bool
scan_down(std::ranges::range auto &&lines, std::size_t row, std::size_t col) {
  return row + 3 < lines.size() && lines[row][col] == 'X'
         && lines[row + 1][col] == 'M' && lines[row + 2][col] == 'A'
         && lines[row + 3][col] == 'S';
}

bool
scan_up(std::ranges::range auto &&lines, std::size_t row, std::size_t col) {
  return row >= 3 && lines[row][col] == 'X' && lines[row - 1][col] == 'M'
         && lines[row - 2][col] == 'A' && lines[row - 3][col] == 'S';
}

bool
scan_down_right(std::ranges::range auto &&lines,
                std::size_t row,
                std::size_t col) {
  return row + 3 < lines.size() && col + 3 < lines[row].size()
         && lines[row][col] == 'X' && lines[row + 1][col + 1] == 'M'
         && lines[row + 2][col + 2] == 'A' && lines[row + 3][col + 3] == 'S';
}

bool
scan_up_right(std::ranges::range auto &&lines, std::size_t row, std::size_t col) {
  return row >= 3 && col + 3 < lines[row].size() && lines[row][col] == 'X'
         && lines[row - 1][col + 1] == 'M' && lines[row - 2][col + 2] == 'A'
         && lines[row - 3][col + 3] == 'S';
}

bool
scan_down_left(std::ranges::range auto &&lines, std::size_t row, std::size_t col) {
  return row + 3 < lines.size() && col >= 3 && lines[row][col] == 'X'
         && lines[row + 1][col - 1] == 'M' && lines[row + 2][col - 2] == 'A'
         && lines[row + 3][col - 3] == 'S';
}

bool
scan_up_left(std::ranges::range auto &&lines, std::size_t row, std::size_t col) {
  return row >= 3 && col >= 3 && lines[row][col] == 'X'
         && lines[row - 1][col - 1] == 'M' && lines[row - 2][col - 2] == 'A'
         && lines[row - 3][col - 3] == 'S';
}

std::uint64_t
count_xmas(std::ranges::range auto &&lines) {
  std::size_t count{};
  for (std::size_t row = 0; row < lines.size(); ++row) {
    auto &&line = lines[row];
    for (std::size_t col = 0; col < line.size(); ++col) {
      if (scan_right(lines, row, col)) {
        ++count;
      }
      if (scan_left(lines, row, col)) {
        ++count;
      }
      if (scan_up(lines, row, col)) {
        ++count;
      }
      if (scan_down(lines, row, col)) {
        ++count;
      }
      if (scan_up_right(lines, row, col)) {
        ++count;
      }
      if (scan_down_right(lines, row, col)) {
        ++count;
      }
      if (scan_up_left(lines, row, col)) {
        ++count;
      }
      if (scan_down_left(lines, row, col)) {
        ++count;
      }
    }
  }
  return count;
}

void
tests() {
  auto lines = std::vector{{"MMMSXXMASM"sv,
                            "MSAMXMSMSA"sv,
                            "AMXSXMAAMM"sv,
                            "MSAMASMSMX"sv,
                            "XMASAMXAMM"sv,
                            "XXAMMXXAMA"sv,
                            "SMSMSASXSS"sv,
                            "SAXAMASAAA"sv,
                            "MAMMMXMMMM"sv,
                            "MXMXAXMASX"sv}};
  ASSERT(count_xmas(lines) == 18);
}

int
main(int argc, char const *const *argv) {
  tests();

  auto args = std::span(argv, size_t(argc));
  if (args.size() != 2) {
    fmt::println(stderr, "usage: {} input.txt", args[0]);
    return 1;
  }

  std::ifstream infile(argv[1]);
  if (!infile.is_open()) {
    fmt::println(stderr, "couldn't open file {}", argv[1]);
    return 2;
  }

  std::vector<std::string> lines;
  for (std::string line; std::getline(infile, line);) {
    lines.emplace_back(std::move(line));
  }

  fmt::println("{}", count_xmas(lines));
  return 0;
}
