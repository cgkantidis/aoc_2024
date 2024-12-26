#include <array> // std::array
#include <cstdint> // std::uint64_t
#include <fmt/core.h> // fmt::print
#include <fstream> // std::ifstream
#include <libassert/assert.hpp> // ASSERT
#include <queue> // std::priority_queue
#include <ranges> // std::views::enumerate
#include <string> // std::string
#include <unordered_map> // std::unordered_map
#include <vector> // std::vector

#include "matrix.hpp" // Matrix
#include "utility.hpp" // split

namespace
{
void
tests();
std::uint64_t
get_num_cheats(std::ranges::range auto &&lines, std::uint64_t min_save);
std::tuple<Location, Location, Matrix<char>>
parse_map(std::ranges::range auto &&lines);
std::uint64_t
find_shortest_path_length_dij(Location start,
                              Location finish,
                              Matrix<char> &grid);
std::uint64_t
find_shortest_path_length_astar(Location start,
                                Location finish,
                                Matrix<char> &grid);
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

  fmt::println("{}", get_num_cheats(lines, 100));
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
        "#.............#"sv,
        "#.............#"sv,
        "#S............#"sv,
        "#.............#"sv,
        "#.............#"sv,
        "#.............#"sv,
        "#....E........#"sv,
        "#.............#"sv,
        "#.............#"sv,
        "#.............#"sv,
        "#.............#"sv,
        "#.............#"sv,
        "#.............#"sv,
        "###############"sv,
    };
    auto [start, finish, grid] = parse_map(lines);
    auto len_dij = find_shortest_path_length_dij(start, finish, grid);
    auto len_astar = find_shortest_path_length_astar(start, finish, grid);
    ASSERT(len_dij == len_astar);
    ASSERT(len_dij == 8);
  }
  {
    auto const lines = std::array{
        "###############"sv,
        "#.............#"sv,
        "#...#.........#"sv,
        "#S..#.#########"sv,
        "#...#.........#"sv,
        "#############.#"sv,
        "#...........#.#"sv,
        "#....E........#"sv,
        "#...........#.#"sv,
        "#...........#.#"sv,
        "#...........#.#"sv,
        "#...........#.#"sv,
        "#.###########.#"sv,
        "#.............#"sv,
        "###############"sv,
    };
    auto [start, finish, grid] = parse_map(lines);
    auto len_dij = find_shortest_path_length_dij(start, finish, grid);
    auto len_astar = find_shortest_path_length_astar(start, finish, grid);
    ASSERT(len_dij == len_astar);
    ASSERT(len_dij == 28);
  }
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
    auto len_astar = find_shortest_path_length_astar(start, finish, grid);
    ASSERT(len_dij == len_astar);
    ASSERT(len_dij == 84);
    ASSERT(get_num_cheats(lines, 1) == 44);
    ASSERT(get_num_cheats(lines, 2) == 44);
    ASSERT(get_num_cheats(lines, 4) == 30);
    ASSERT(get_num_cheats(lines, 6) == 16);
    ASSERT(get_num_cheats(lines, 8) == 14);
    ASSERT(get_num_cheats(lines, 10) == 10);
    ASSERT(get_num_cheats(lines, 12) == 8);
    ASSERT(get_num_cheats(lines, 20) == 5);
    ASSERT(get_num_cheats(lines, 36) == 4);
    ASSERT(get_num_cheats(lines, 38) == 3);
    ASSERT(get_num_cheats(lines, 40) == 2);
    ASSERT(get_num_cheats(lines, 64) == 1);
  }
}

std::uint64_t
get_num_cheats(std::ranges::range auto &&lines, std::uint64_t min_save) {
  auto [start, finish, grid] = parse_map(lines);
  auto min1 = find_shortest_path_length_dij(start, finish, grid);
  auto min2 = find_shortest_path_length_astar(start, finish, grid);
  ASSERT(min1 == min2);

  std::uint64_t num_cheats{};
  // horizontal domino
  for (std::size_t row{1}; row < grid.rows() - 1; ++row) {
    for (std::size_t col{1}; col < grid.cols() - 1; ++col) {
      if (grid(row, col) != '#') {
        continue;
      }

      grid(row, col) = ' ';
      //auto cost1 = find_shortest_path_length_dij(start, finish, grid);
      reset_grid(start, finish, grid);
      auto cost2 = find_shortest_path_length_astar(start, finish, grid);
      //ASSERT(cost1 == cost2);
      ASSERT(cost2 <= min1);
      if (min1 - cost2 >= min_save) {
        ++num_cheats;
        fmt::println("{},{}", row, col);
      }
      grid(row, col) = '#';
    }
  }
  return num_cheats;
}

std::tuple<Location, Location, Matrix<char>>
parse_map(std::ranges::range auto &&lines) {
  Location start{};
  Location finish{};
  Matrix<char> grid(lines.size() + 2, lines[0].size() + 2, ' ');

  for (std::size_t row{}; row < grid.rows(); ++row) {
    grid(row, 0) = grid(row, grid.cols() - 1) = '#';
  }
  for (std::size_t col{}; col < grid.cols(); ++col) {
    grid(0, col) = grid(grid.rows() - 1, col) = '#';
  }

  for (auto const [row, line] : std::views::enumerate(lines)) {
    for (auto const [col, ch] : std::views::enumerate(line)) {
      switch (ch) {
      case '#': {
        grid(size_t(row + 1), size_t(col + 1)) = '#';
        break;
      }
      case 'S': {
        start = Location{size_t(row + 1), size_t(col + 1)};
        grid(size_t(row + 1), size_t(col + 1)) = 'S';
        break;
      }
      case 'E': {
        finish = Location{size_t(row + 1), size_t(col + 1)};
        grid(size_t(row + 1), size_t(col + 1)) = 'E';
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

std::uint64_t
find_shortest_path_length_dij(Location start,
                              Location finish,
                              Matrix<char> &grid) {
  std::unordered_map<Location, std::uint64_t> cost_map;

  using State = std::pair<Location, std::uint64_t>;
  auto cmp = [](State const &lhs, State const &rhs) {
    return lhs.second > rhs.second;
  };
  std::priority_queue<State, std::vector<State>, decltype(cmp)> to_visit(cmp);
  to_visit.emplace(start, 0);

  while (!to_visit.empty()) {
    auto [top, cost] = to_visit.top();
    auto [row, col] = top;
    to_visit.pop();

    // mark as visited
    cost_map[top] = cost;
    // mark the grid
    grid(row, col) = '.';

    if (top == finish) {
      return cost;
    }

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

  return std::numeric_limits<std::uint64_t>::max(); // no path
}

std::uint64_t
manhatan_distance(Location loc, Location finish) {
  return (loc.row > finish.row ? loc.row - finish.row : finish.row - loc.row)
         + (loc.col > finish.col ? loc.col - finish.col : finish.col - loc.col);
}

std::uint64_t
find_shortest_path_length_astar(Location start,
                                Location finish,
                                Matrix<char> &grid) {
  std::unordered_map<Location, std::uint64_t> cost_map;

  using State = std::tuple<Location, std::uint64_t, std::uint64_t>;
  auto cmp = [](State const &lhs, State const &rhs) {
    return std::get<1>(lhs) + std::get<2>(lhs)
           > std::get<1>(rhs) + std::get<2>(rhs);
  };
  std::priority_queue<State, std::vector<State>, decltype(cmp)> to_visit(cmp);
  to_visit.emplace(start, 0, manhatan_distance(start, finish));

  while (!to_visit.empty()) {
    auto [top, cost, heur] = to_visit.top();
    auto [row, col] = top;
    to_visit.pop();

    // mark as visited
    cost_map[top] = cost;
    // mark the grid
    grid(row, col) = '.';

    if (top == finish) {
      return cost;
    }

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
      to_visit.emplace(nloc, cost + 1, manhatan_distance(nloc, finish));
    }
  }

  return std::numeric_limits<std::uint64_t>::max(); // no path
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
