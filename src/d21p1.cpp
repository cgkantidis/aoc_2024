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
void
append_range(std::vector<char> &dst, std::vector<char> const &src);
std::vector<std::vector<char>>
all_shortest_paths(char src,
                   char dst,
                   Matrix<char> const &keypad1,
                   Matrix<char> const &keypad2,
                   u64 depth);
std::pair<std::vector<char>, u64>
get_path_with_min_cost(char src,
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
  auto const keypad1 = parse_keypad(keypad1_lines);
  auto const keypad2 = parse_keypad(keypad2_lines);

  ASSERT(get_path_with_min_cost('A', '^', keypad1, keypad2, 0).second == 2);
  ASSERT(get_path_with_min_cost('A', '>', keypad1, keypad2, 0).second == 2);
  ASSERT(get_path_with_min_cost('A', 'v', keypad1, keypad2, 0).second == 3);
  ASSERT(get_path_with_min_cost('A', '<', keypad1, keypad2, 0).second == 4);
  ASSERT(get_path_with_min_cost('>', 'A', keypad1, keypad2, 0).second == 2);
  ASSERT(get_path_with_min_cost('>', '^', keypad1, keypad2, 0).second == 3);
  ASSERT(get_path_with_min_cost('>', 'v', keypad1, keypad2, 0).second == 2);
  ASSERT(get_path_with_min_cost('>', '<', keypad1, keypad2, 0).second == 3);
  ASSERT(get_path_with_min_cost('<', 'A', keypad1, keypad2, 0).second == 4);
  ASSERT(get_path_with_min_cost('<', '^', keypad1, keypad2, 0).second == 3);
  ASSERT(get_path_with_min_cost('<', 'v', keypad1, keypad2, 0).second == 2);
  ASSERT(get_path_with_min_cost('<', '>', keypad1, keypad2, 0).second == 3);
  ASSERT(get_path_with_min_cost('v', 'A', keypad1, keypad2, 0).second == 3);
  ASSERT(get_path_with_min_cost('v', '^', keypad1, keypad2, 0).second == 2);
  ASSERT(get_path_with_min_cost('v', '<', keypad1, keypad2, 0).second == 2);
  ASSERT(get_path_with_min_cost('v', '>', keypad1, keypad2, 0).second == 2);
  ASSERT(get_path_with_min_cost('^', 'A', keypad1, keypad2, 0).second == 2);
  ASSERT(get_path_with_min_cost('^', 'v', keypad1, keypad2, 0).second == 2);
  ASSERT(get_path_with_min_cost('^', '<', keypad1, keypad2, 0).second == 3);
  ASSERT(get_path_with_min_cost('^', '>', keypad1, keypad2, 0).second == 3);
  ASSERT(get_path_with_min_cost('A', '^', keypad1, keypad2, 1).second == 8);
  ASSERT(get_path_with_min_cost('A', '>', keypad1, keypad2, 1).second == 6);
  ASSERT(get_path_with_min_cost('A', 'v', keypad1, keypad2, 1).second == 9);
  ASSERT(get_path_with_min_cost('A', '<', keypad1, keypad2, 1).second == 10);
  ASSERT(get_path_with_min_cost('>', 'A', keypad1, keypad2, 1).second == 4);
  ASSERT(get_path_with_min_cost('>', '^', keypad1, keypad2, 1).second == 9);
  ASSERT(get_path_with_min_cost('>', 'v', keypad1, keypad2, 1).second == 8);
  ASSERT(get_path_with_min_cost('>', '<', keypad1, keypad2, 1).second == 9);
  ASSERT(get_path_with_min_cost('<', 'A', keypad1, keypad2, 1).second == 8);
  ASSERT(get_path_with_min_cost('<', '^', keypad1, keypad2, 1).second == 7);
  ASSERT(get_path_with_min_cost('<', 'v', keypad1, keypad2, 1).second == 4);
  ASSERT(get_path_with_min_cost('<', '>', keypad1, keypad2, 1).second == 5);
  ASSERT(get_path_with_min_cost('v', 'A', keypad1, keypad2, 1).second == 7);
  ASSERT(get_path_with_min_cost('v', '^', keypad1, keypad2, 1).second == 4);
  ASSERT(get_path_with_min_cost('v', '<', keypad1, keypad2, 1).second == 8);
  ASSERT(get_path_with_min_cost('v', '>', keypad1, keypad2, 1).second == 4);
  ASSERT(get_path_with_min_cost('^', 'A', keypad1, keypad2, 1).second == 4);
  ASSERT(get_path_with_min_cost('^', 'v', keypad1, keypad2, 1).second == 6);
  ASSERT(get_path_with_min_cost('^', '<', keypad1, keypad2, 1).second == 9);
  ASSERT(get_path_with_min_cost('^', '>', keypad1, keypad2, 1).second == 7);
  ASSERT(get_path_with_min_cost('A', '3', keypad1, keypad2, 2).second == 12);
  ASSERT(get_path_with_min_cost('3', '7', keypad1, keypad2, 2).second == 23);
  ASSERT(get_path_with_min_cost('7', '9', keypad1, keypad2, 2).second == 11);
  ASSERT(get_path_with_min_cost('9', 'A', keypad1, keypad2, 2).second == 18);
  ASSERT(get_path_with_min_cost('4', '0', keypad1, keypad2, 2).second == 22);

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
  return get_path_with_min_cost('A', line[0], keypad1, keypad2, 2).second
         + std::ranges::fold_left(
             line | std::views::slide(2),
             0ULL,
             [&keypad1, &keypad2](u64 prev, auto const &window) {
               return prev
                      + get_path_with_min_cost(window[0],
                                               window[1],
                                               keypad1,
                                               keypad2,
                                               2)
                            .second;
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

std::vector<std::vector<char>>
all_shortest_paths(char src,
                   char dst,
                   Matrix<char> const &keypad1,
                   Matrix<char> const &keypad2,
                   u64 depth) {
  auto keypad = depth == 2 ? keypad1 : keypad2;

  std::unordered_map<Location, std::vector<std::vector<char>>> shortest_paths;
  std::queue<std::pair<Location, std::vector<char>>> to_visit;
  to_visit.emplace(find_key(src, keypad), std::vector<char>{});

  while (!to_visit.empty()) {
    auto [loc, moves] = to_visit.front();
    auto [row, col] = loc;
    to_visit.pop();

    auto find_it = shortest_paths.find(loc);
    if (find_it != shortest_paths.end()) {
      if (moves.size() < find_it->second.front().size()) {
        // we found a new shortest path
        find_it->second.clear();
        find_it->second.emplace_back(moves);
      } else if (moves.size() == find_it->second.front().size()) {
        // we found another shortest path
        find_it->second.emplace_back(moves);
      } else {
        continue;
      }
    } else {
      shortest_paths[loc] = {moves};
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
      to_visit.emplace(nloc, nmoves);
    }
  }
  return shortest_paths.find(find_key(dst, keypad))->second;
}

std::pair<std::vector<char>, u64>
get_path_with_min_cost(char src,
                       char dst,
                       Matrix<char> const &keypad1,
                       Matrix<char> const &keypad2,
                       u64 depth) {
  auto paths = all_shortest_paths(src, dst, keypad1, keypad2, depth);
  if (depth == 0) {
    std::vector<char> tmp_path = paths.front();
    tmp_path.emplace_back('A');
    return {tmp_path, tmp_path.size()};
  }

  u64 min_cost = std::numeric_limits<u64>::max();
  std::vector<char> path_with_min_cost;
  for (auto const &path : paths) {
    u64 cost = 0;
    std::vector<char> tmp_path{'A'};
    append_range(tmp_path, path);
    tmp_path.emplace_back('A');

    for (auto const &window : tmp_path | std::views::slide(2)) {
      cost +=
          get_path_with_min_cost(window[0], window[1], keypad1, keypad2, depth - 1)
              .second;
    }
    if (cost < min_cost) {
      min_cost = cost;
      path_with_min_cost = path;
    }
  }

  return {path_with_min_cost, min_cost};
}
} // namespace
