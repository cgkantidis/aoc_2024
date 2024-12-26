#include <array> // std::array
#include <cstdint> // std::uint64_t
#include <fmt/core.h> // fmt::print
#include <fstream> // std::ifstream
#include <functional> // std::plus
#include <libassert/assert.hpp> // ASSERT
#include <queue> // std::priority_queue
#include <ranges> // std::views::enumerate
#include <string> // std::string
#include <thread> // std::thread
#include <unordered_map> // std::unordered_map
#include <unordered_set> // std::unordered_set
#include <vector> // std::vector

#include "matrix.hpp" // Matrix
#include "utility.hpp" // split

using State = std::pair<Location, std::uint64_t>;
auto constexpr cmp = [](State const &lhs, State const &rhs) {
  return lhs.second > rhs.second;
};
using pq = std::priority_queue<State, std::vector<State>, decltype(cmp)>;

namespace
{
void
tests();
std::uint64_t
get_num_cheats(std::ranges::range auto &&lines,
               std::uint64_t min_save,
               std::uint64_t max_cheat);
std::tuple<Location, Location, Matrix<char>>
parse_map(std::ranges::range auto &&lines);
std::pair<std::uint64_t, std::unordered_map<Location, std::uint64_t>>
find_shortest_path_length_dij(Location start,
                              Location finish,
                              Matrix<char> const &grid);
void
find_shortest_path_length_dij_with_cheat(
    Location finish,
    Matrix<char> &grid,
    std::unordered_map<Location, std::uint64_t> const &cost_map,
    std::size_t cl /*cheat_length*/,
    std::uint64_t max_cost,
    std::uint64_t min_save,
    std::uint64_t &num_cheats);
std::uint64_t
find_shortest_path_length_dij_with_cheat_int(
    Location finish,
    Matrix<char> const &grid,
    std::unordered_map<Location, std::uint64_t> cost_map,
    pq &to_visit);
void
reset_grid(Location start, Location finish, Matrix<char> &grid);
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

  fmt::println("{}", get_num_cheats(lines, 100, 20));
  return 0;
}

namespace
{
void
tests() {
  using namespace std::literals::string_view_literals;
  {
    auto const lines = std::array{
        "###############"sv,
        "#...#...#.....#"sv,
        "#.#.#.#.#.###.#"sv,
        "#S#...#.#.#...#"sv,
        "#######.#.#.###"sv,
        "#######.#.#...#"sv,
        "#######.#.###.#"sv,
        "###..E#...#...#"sv,
        "###.#######.###"sv,
        "#...###...#...#"sv,
        "#.#####.#.###.#"sv,
        "#.#...#.#.#...#"sv,
        "#.#.#.#.#.#.###"sv,
        "#...#...#...###"sv,
        "###############"sv,
    };
    auto [start, finish, grid] = parse_map(lines);
    auto len_dij = find_shortest_path_length_dij(start, finish, grid);
    ASSERT(len_dij.first == 84);
    ASSERT(get_num_cheats(lines, 64, 1) == 1);
    ASSERT(get_num_cheats(lines, 40, 1) == 2);
    ASSERT(get_num_cheats(lines, 38, 1) == 3);
    ASSERT(get_num_cheats(lines, 36, 1) == 4);
    ASSERT(get_num_cheats(lines, 20, 1) == 5);
    ASSERT(get_num_cheats(lines, 12, 1) == 8);
    ASSERT(get_num_cheats(lines, 10, 1) == 10);
    ASSERT(get_num_cheats(lines, 8, 1) == 14);
    ASSERT(get_num_cheats(lines, 6, 1) == 16);
    ASSERT(get_num_cheats(lines, 4, 1) == 30);
    ASSERT(get_num_cheats(lines, 2, 1) == 44);
    ASSERT(get_num_cheats(lines, 1, 1) == 44);

    ASSERT(get_num_cheats(lines, 76, 20) == 3);
    ASSERT(get_num_cheats(lines, 74, 20) == 3 + 4);
    ASSERT(get_num_cheats(lines, 72, 20) == 3 + 4 + 22);
    ASSERT(get_num_cheats(lines, 70, 20) == 3 + 4 + 22 + 12);
    ASSERT(get_num_cheats(lines, 68, 20) == 3 + 4 + 22 + 12 + 14);
    ASSERT(get_num_cheats(lines, 66, 20) == 3 + 4 + 22 + 12 + 14 + 12);
    ASSERT(get_num_cheats(lines, 64, 20) == 3 + 4 + 22 + 12 + 14 + 12 + 19);
    ASSERT(get_num_cheats(lines, 62, 20) == 3 + 4 + 22 + 12 + 14 + 12 + 19 + 20);
    ASSERT(get_num_cheats(lines, 60, 20)
           == 3 + 4 + 22 + 12 + 14 + 12 + 19 + 20 + 23);
    ASSERT(get_num_cheats(lines, 58, 20)
           == 3 + 4 + 22 + 12 + 14 + 12 + 19 + 20 + 23 + 25);
    ASSERT(get_num_cheats(lines, 56, 20)
           == 3 + 4 + 22 + 12 + 14 + 12 + 19 + 20 + 23 + 25 + 39);
    ASSERT(get_num_cheats(lines, 54, 20)
           == 3 + 4 + 22 + 12 + 14 + 12 + 19 + 20 + 23 + 25 + 39 + 29);
    ASSERT(get_num_cheats(lines, 52, 20)
           == 3 + 4 + 22 + 12 + 14 + 12 + 19 + 20 + 23 + 25 + 39 + 29 + 31);
    ASSERT(get_num_cheats(lines, 50, 20)
           == 3 + 4 + 22 + 12 + 14 + 12 + 19 + 20 + 23 + 25 + 39 + 29 + 31 + 32);
  }
}

std::uint64_t
get_num_cheats(std::ranges::range auto &&lines,
               std::uint64_t min_save,
               std::uint64_t max_cheat) {
  auto [start, finish, grid] = parse_map(lines);
  auto [max_cost, cost_map] = find_shortest_path_length_dij(start, finish, grid);

  std::vector<std::uint64_t> num_cheats(max_cheat);
  std::vector<std::thread> threads;
  for (std::uint64_t cl = 1; cl <= max_cheat; ++cl) {
    threads.emplace_back(find_shortest_path_length_dij_with_cheat,
                         finish,
                         std::ref(grid),
                         std::ref(cost_map),
                         cl + 1,
                         max_cost,
                         min_save,
                         std::ref(num_cheats[cl - 1]));
  }
  std::uint64_t cl = 1;
  for (std::thread &t : threads) {
    fmt::println("{}", cl++);
    t.join();
  }
  return std::ranges::fold_left(num_cheats, 0ULL, std::plus<std::uint64_t>{});
}

std::tuple<Location, Location, Matrix<char>>
parse_map(std::ranges::range auto &&lines) {
  Location start{};
  Location finish{};
  Matrix<char> grid(lines.size(), lines[0].size(), ' ');

  for (auto const [row, line] : std::views::enumerate(lines)) {
    for (auto const [col, ch] : std::views::enumerate(line)) {
      switch (ch) {
      case '#': {
        grid(size_t(row), size_t(col)) = '#';
        break;
      }
      case 'S': {
        start = Location{size_t(row), size_t(col)};
        grid(size_t(row), size_t(col)) = 'S';
        break;
      }
      case 'E': {
        finish = Location{size_t(row), size_t(col)};
        grid(size_t(row), size_t(col)) = 'E';
        break;
      }
      case '.':
        break;
      default:
        UNREACHABLE(ch);
      }
    }
  }
  return {start, finish, grid};
}

std::pair<std::uint64_t, std::unordered_map<Location, std::uint64_t>>
find_shortest_path_length_dij(Location start,
                              Location finish,
                              Matrix<char> const &grid) {
  std::unordered_map<Location, std::uint64_t> cost_map;
  pq to_visit(cmp);
  to_visit.emplace(start, 0);

  while (!to_visit.empty()) {
    auto [top, cost] = to_visit.top();
    auto [row, col] = top;
    to_visit.pop();

    // mark as visited
    cost_map[top] = cost;

    for (auto nloc : {Location{row - 1, col},
                      Location{row + 1, col},
                      Location{row, col - 1},
                      Location{row, col + 1}}) {
      auto [nrow, ncol] = nloc;
      if (grid(nrow, ncol) == '#') {
        continue;
      }
      if (cost_map.find(nloc) != cost_map.end()) {
        continue; // already visited
      }
      to_visit.emplace(nloc, cost + 1);
    }
  }

  auto find_it = cost_map.find(finish);
  if (find_it != cost_map.end()) {
    return {find_it->second, cost_map};
  }
  UNREACHABLE("there should always be a path");
}

void
find_shortest_path_length_dij_with_cheat(
    Location finish,
    Matrix<char> &grid,
    std::unordered_map<Location, std::uint64_t> const &cost_map,
    std::size_t cl /*cheat_length*/,
    std::uint64_t max_cost,
    std::uint64_t min_save,
    std::uint64_t &num_cheats) {
  pq to_visit(cmp);
  for (std::size_t row{1}; row < grid.rows() - 1; ++row) {
    for (std::size_t col{1}; col < grid.cols() - 1; ++col) {
      if (grid(row, col) == '#') {
        continue;
      }
      std::unordered_set<Location> ces;
      Location cs{row, col};
      for (std::size_t hc = 0; hc <= cl; ++hc) { // horizontal cheat
        std::size_t vc = cl - hc; // vertical cheat
        if (cs.row >= vc && cs.col >= hc) {
          if (grid(cs.row - vc, cs.col - hc) != '#') {
            ces.emplace(cs.row - vc, cs.col - hc);
          }
        }
        if (cs.row >= vc && cs.col + hc < grid.cols()) {
          if (grid(cs.row - vc, cs.col + hc) != '#') {
            ces.emplace(cs.row - vc, cs.col + hc);
          }
        }
        if (cs.row + vc < grid.rows() && cs.col >= hc) {
          if (grid(cs.row + vc, cs.col - hc) != '#') {
            ces.emplace(cs.row + vc, cs.col - hc);
          }
        }
        if (cs.row + vc < grid.rows() && cs.col + hc < grid.cols()) {
          if (grid(cs.row + vc, cs.col + hc) != '#') {
            ces.emplace(cs.row + vc, cs.col + hc);
          }
        }
      }

      for (Location const &ce : ces) {
        ASSERT(to_visit.empty());
        to_visit.emplace(ce, cost_map.find(cs)->second + cl);
        auto cost = find_shortest_path_length_dij_with_cheat_int(finish,
                                                                 grid,
                                                                 cost_map,
                                                                 to_visit);
        ASSERT(cost <= max_cost);
        if (max_cost - cost >= min_save) {
          ++num_cheats;
        }
      }
    }
  }
}

std::uint64_t
find_shortest_path_length_dij_with_cheat_int(
    Location finish,
    Matrix<char> const &grid,
    std::unordered_map<Location, std::uint64_t> cost_map,
    pq &to_visit) {
  while (!to_visit.empty()) {
    auto [top, cost] = to_visit.top();
    auto [row, col] = top;
    to_visit.pop();

    if (top == finish) {
      while (!to_visit.empty()) {
        to_visit.pop();
      }
      return cost;
    }

    if (cost >= cost_map[top]) {
      continue;
    }

    // mark as visited
    cost_map[top] = cost;

    for (auto nloc : {Location{row - 1, col},
                      Location{row + 1, col},
                      Location{row, col - 1},
                      Location{row, col + 1}}) {
      auto [nrow, ncol] = nloc;
      if (grid(nrow, ncol) == '#') {
        continue;
      }
      auto find_it = cost_map.find(nloc);
      if (find_it == cost_map.end() || find_it->second > cost + 1) {
        to_visit.emplace(nloc, cost + 1);
      }
    }
  }

  auto find_it = cost_map.find(finish);
  if (find_it != cost_map.end()) {
    return find_it->second;
  }
  UNREACHABLE("there should always be a path");
}

void
reset_grid(Location start, Location finish, Matrix<char> &grid) {
  for (std::size_t row{}; row < grid.rows(); ++row) {
    for (std::size_t col{}; col < grid.cols(); ++col) {
      if (grid(row, col) != '#') {
        grid(row, col) = ' ';
      }
    }
  }
  grid(start.row, start.col) = 'S';
  grid(finish.row, finish.col) = 'E';
}
} // namespace
