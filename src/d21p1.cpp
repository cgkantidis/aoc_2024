#include <array> // std::array
#include <cstdint> // u64
#include <fmt/core.h> // fmt::print
#include <fmt/ranges.h> // fmt::print
#include <fstream> // std::ifstream
#include <libassert/assert.hpp> // ASSERT
#include <queue> // std::priority_queue
#include <ranges> // std::views::enumerate
#include <string> // std::string
#include <unordered_map> // std::unordered_map
#include <vector> // std::vector

#include "matrix.hpp" // Matrix
#include "utility.hpp" // split

using u64 = std::uint64_t;
struct State
{
  Location dst;
  std::vector<char> moves;
  u64 lower_level_cost;
};

using namespace std::literals::string_view_literals;

auto constexpr keypad1_lines = std::array{
    "#####"sv,
    "#789#"sv,
    "#456#"sv,
    "#123#"sv,
    "##0A#"sv,
    "#####"sv,
};

auto constexpr keypad2_lines = std::array{
    "#####"sv,
    "##^A#"sv,
    "#<v>#"sv,
    "#####"sv,
};

namespace
{
void
tests();
u64
get_sum_complexities(std::ranges::range auto &&lines);
u64
get_num_moves(std::string_view line,
              Matrix<char> const &keypad1,
              Matrix<char> const &keypad2);
Matrix<char>
parse_keypad(std::ranges::range auto &&lines);
std::vector<char>
moves_for_path(char src,
               char dst,
               Matrix<char> const &keypad1,
               Matrix<char> const &keypad2,
               u64 depth);
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

  fmt::println("{}", get_sum_complexities(lines));
  return 0;
}

namespace
{
void
tests() {
  {
    auto const lines = std::array{
        "029A"sv,
        "980A"sv,
        "179A"sv,
        "456A"sv,
        "379A"sv,
    };
    ASSERT(get_sum_complexities(lines) == 126384);
  }
  auto const keypad1 = parse_keypad(keypad1_lines);
  auto const keypad2 = parse_keypad(keypad2_lines);
  ASSERT(moves_for_path('A', '^', keypad1, keypad2, 0).size() == 2);
  ASSERT(moves_for_path('A', '>', keypad1, keypad2, 0).size() == 2);
  ASSERT(moves_for_path('A', 'v', keypad1, keypad2, 0).size() == 3);
  ASSERT(moves_for_path('A', '<', keypad1, keypad2, 0).size() == 4);
  ASSERT(moves_for_path('>', 'A', keypad1, keypad2, 0).size() == 2);
  ASSERT(moves_for_path('>', '^', keypad1, keypad2, 0).size() == 3);
  ASSERT(moves_for_path('>', 'v', keypad1, keypad2, 0).size() == 2);
  ASSERT(moves_for_path('>', '<', keypad1, keypad2, 0).size() == 3);
  ASSERT(moves_for_path('<', 'A', keypad1, keypad2, 0).size() == 4);
  ASSERT(moves_for_path('<', '^', keypad1, keypad2, 0).size() == 3);
  ASSERT(moves_for_path('<', 'v', keypad1, keypad2, 0).size() == 2);
  ASSERT(moves_for_path('<', '>', keypad1, keypad2, 0).size() == 3);
  ASSERT(moves_for_path('v', 'A', keypad1, keypad2, 0).size() == 3);
  ASSERT(moves_for_path('v', '^', keypad1, keypad2, 0).size() == 2);
  ASSERT(moves_for_path('v', '<', keypad1, keypad2, 0).size() == 2);
  ASSERT(moves_for_path('v', '>', keypad1, keypad2, 0).size() == 2);
  ASSERT(moves_for_path('^', 'A', keypad1, keypad2, 0).size() == 2);
  ASSERT(moves_for_path('^', 'v', keypad1, keypad2, 0).size() == 2);
  ASSERT(moves_for_path('^', '<', keypad1, keypad2, 0).size() == 3);
  ASSERT(moves_for_path('^', '>', keypad1, keypad2, 0).size() == 3);

  ASSERT(moves_for_path('A', '^', keypad1, keypad2, 1).size() == 8);
  ASSERT(moves_for_path('A', '>', keypad1, keypad2, 1).size() == 6);
  ASSERT(moves_for_path('A', 'v', keypad1, keypad2, 1).size() == 9);
  ASSERT(moves_for_path('A', '<', keypad1, keypad2, 1).size() == 10);
  ASSERT(moves_for_path('>', 'A', keypad1, keypad2, 1).size() == 4);
  ASSERT(moves_for_path('>', '^', keypad1, keypad2, 1).size() == 9);
  ASSERT(moves_for_path('>', 'v', keypad1, keypad2, 1).size() == 8);
  ASSERT(moves_for_path('>', '<', keypad1, keypad2, 1).size() == 9);
  ASSERT(moves_for_path('<', 'A', keypad1, keypad2, 1).size() == 8);
  ASSERT(moves_for_path('<', '^', keypad1, keypad2, 1).size() == 7);
  ASSERT(moves_for_path('<', 'v', keypad1, keypad2, 1).size() == 4);
  ASSERT(moves_for_path('<', '>', keypad1, keypad2, 1).size() == 5);
  ASSERT(moves_for_path('v', 'A', keypad1, keypad2, 1).size() == 7);
  ASSERT(moves_for_path('v', '^', keypad1, keypad2, 1).size() == 4);
  ASSERT(moves_for_path('v', '<', keypad1, keypad2, 1).size() == 8);
  ASSERT(moves_for_path('v', '>', keypad1, keypad2, 1).size() == 4);
  ASSERT(moves_for_path('^', 'A', keypad1, keypad2, 1).size() == 4);
  ASSERT(moves_for_path('^', 'v', keypad1, keypad2, 1).size() == 6);
  ASSERT(moves_for_path('^', '<', keypad1, keypad2, 1).size() == 9);
  ASSERT(moves_for_path('^', '>', keypad1, keypad2, 1).size() == 7);

  ASSERT(moves_for_path('A', '3', keypad1, keypad2, 2).size() == 12);
  ASSERT(moves_for_path('3', '7', keypad1, keypad2, 2).size() == 23);
  ASSERT(moves_for_path('7', '9', keypad1, keypad2, 2).size() == 11);
  ASSERT(moves_for_path('9', 'A', keypad1, keypad2, 2).size() == 18);
}

u64
get_sum_complexities(std::ranges::range auto &&lines) {
  auto const keypad1 = parse_keypad(keypad1_lines);
  auto const keypad2 = parse_keypad(keypad2_lines);

  return std::ranges::fold_left(
      lines,
      0ULL,
      [&keypad1, &keypad2](u64 prev, std::string_view line) {
        return prev
               + str_to_int<u64>(line.substr(0, line.size() - 1))
                     * get_num_moves(line, keypad1, keypad2);
      });
}

u64
get_num_moves(std::string_view line,
              Matrix<char> const &keypad1,
              Matrix<char> const &keypad2) {
  return moves_for_path('A', line[0], keypad1, keypad2, 2).size()
         + std::ranges::fold_left(
             line | std::views::slide(2),
             0ULL,
             [&keypad1, &keypad2](u64 prev, auto const &window) {
               return prev
                      + moves_for_path(window[0], window[1], keypad1, keypad2, 2)
                            .size();
             });
}

Location
find_key(char key, Matrix<char> const &keypad) {
  for (std::size_t row{}; row < keypad.rows(); ++row) {
    for (std::size_t col{}; col < keypad.cols(); ++col) {
      if (keypad(row, col) == key) {
        return {row, col};
      }
    }
  }
  UNREACHABLE("key not found", key);
}

Matrix<char>
parse_keypad(std::ranges::range auto &&lines) {
  Matrix<char> grid(lines.size(), lines[0].size(), ' ');
  for (auto const [row, line] : std::views::enumerate(lines)) {
    for (auto const [col, ch] : std::views::enumerate(line)) {
      grid(row, col) = ch;
    }
  }
  return grid;
}

void
append_range(std::vector<char> &dst, std::vector<char> const &src) {
  dst.insert(dst.end(), src.cbegin(), src.cend());
}

std::vector<char>
moves_for_path(char src,
               char dst,
               Matrix<char> const &keypad1,
               Matrix<char> const &keypad2,
               u64 depth) {
  auto keypad = depth == 2 ? keypad1 : keypad2;

  std::unordered_map<Location, std::pair<std::vector<char>, u64>> move_map;
  std::queue<State> to_visit;
  to_visit.emplace(find_key(src, keypad), std::vector<char>());

  while (!to_visit.empty()) {
    auto [top, moves, cost] = to_visit.front();
    auto [row, col] = top;
    to_visit.pop();

    // mark as visited
    auto find_it = move_map.find(top);
    if (find_it != move_map.end()) {
      if (find_it->second.second < cost) {
        continue;
      }
      if (find_it->second.second > cost) {
        move_map[top] = {moves, cost};
      }
    } else {
      move_map[top] = {moves, cost};
    }

    for (auto [nloc, nmove] : {
             std::make_pair(Location{row - 1, col}, '^'),
             std::make_pair(Location{row + 1, col}, 'v'),
             std::make_pair(Location{row, col - 1}, '<'),
             std::make_pair(Location{row, col + 1}, '>'),
         }) {
      if (keypad(nloc) == '#') {
        continue;
      }

      auto nmoves = moves;
      nmoves.emplace_back(nmove);

      if (depth == 0) {
        to_visit.emplace(nloc, nmoves, nmoves.size());
      } else {
        auto tmp_moves = std::vector{'A'};
        append_range(tmp_moves, nmoves);
        tmp_moves.emplace_back('A');

        std::vector<char> d0_moves;
        for (auto const &window : tmp_moves | std::views::slide(2)) {
          append_range(
              d0_moves,
              moves_for_path(window[0], window[1], keypad1, keypad2, depth - 1));
        }
        to_visit.emplace(nloc, nmoves, d0_moves.size());
      }
    }
  }

  auto find_it = move_map.find(find_key(dst, keypad));
  find_it->second.first.emplace_back('A');
  if (depth == 0) {
    return find_it->second.first;
  }
  auto tmp_moves = std::vector{'A'};
  append_range(tmp_moves, find_it->second.first);
  std::vector<char> d0_moves;
  for (auto const &window : tmp_moves | std::views::slide(2)) {
    append_range(
        d0_moves,
        moves_for_path(window[0], window[1], keypad1, keypad2, depth - 1));
  }
  return d0_moves;
}
} // namespace
