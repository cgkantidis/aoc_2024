#include <fmt/core.h>
#include <fstream>
#include <ranges>
#include <string>
#include <string_view>

#include <libassert/assert.hpp>

using namespace std::literals::string_view_literals;

bool
scan_diag1(std::ranges::range auto &&lines, std::size_t row, std::size_t col) {
  return (lines[row - 1][col - 1] == 'M' && lines[row + 1][col + 1] == 'S')
         || (lines[row - 1][col - 1] == 'S' && lines[row + 1][col + 1] == 'M');
}

bool
scan_diag2(std::ranges::range auto &&lines, std::size_t row, std::size_t col) {
  return (lines[row - 1][col + 1] == 'M' && lines[row + 1][col - 1] == 'S')
         || (lines[row - 1][col + 1] == 'S' && lines[row + 1][col - 1] == 'M');
}

std::uint64_t
count_xmas(std::ranges::range auto &&lines) {
  std::size_t count{};
  for (std::size_t row = 1; row < lines.size() - 1; ++row) {
    auto &&line = lines[row];
    for (std::size_t col = 1; col < line.size() - 1; ++col) {
      if (lines[row][col] == 'A' && scan_diag1(lines, row, col)
          && scan_diag2(lines, row, col)) {
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
  ASSERT(count_xmas(lines) == 9);
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
