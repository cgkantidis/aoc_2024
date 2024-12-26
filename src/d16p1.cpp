#include <array>
#include <cstdint>
#include <fmt/core.h>
#include <fstream>
#include <libassert/assert.hpp>
#include <queue>
#include <ranges>
#include <scn/scan.h>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>

#include "matrix.hpp"

enum Direction : unsigned
{
  UP,
  DOWN,
  LEFT,
  RIGHT,
};

char
dir_to_ch(Direction dir) {
  switch (dir) {
  case Direction::UP:
    return '^';
  case Direction::LEFT:
    return '<';
  case Direction::DOWN:
    return 'v';
  case Direction::RIGHT:
    return '>';
  default:
    UNREACHABLE(dir);
  }
}

Direction
rotate_ccw(Direction dir) {
  switch (dir) {
  case Direction::UP:
    return Direction::LEFT;
  case Direction::LEFT:
    return Direction::DOWN;
  case Direction::DOWN:
    return Direction::RIGHT;
  case Direction::RIGHT:
    return Direction::UP;
  default:
    UNREACHABLE(dir);
  }
}

struct State
{
  Location loc;
  Direction dir;
};

/// we need the operator==() to resolve hash collisions
bool
operator==(State const &lhs, State const &rhs) {
  return lhs.loc == rhs.loc && lhs.dir == rhs.dir;
}

/// custom specialization of std::hash injected in namespace std
template <>
struct std::hash<State>
{
  std::size_t
  operator()(State const &s) const noexcept {
    std::size_t h1 = std::hash<std::size_t>{}(s.loc.row);
    std::size_t h2 = std::hash<std::size_t>{}(s.loc.col);
    std::size_t h3 = std::hash<std::size_t>{}(s.dir);

    std::size_t ret_val = 0;
    hash_combine(ret_val, h1, h2, h3);
    return ret_val;
  }
};

namespace
{
void
tests();
std::uint64_t
lowest_score(std::ranges::range auto &&lines);
std::tuple<Location, Location, Matrix<bool>>
parse_map(std::ranges::range auto &&lines);
void
print_grid(Matrix<bool> const &grid);
std::pair<bool, State>
next_state(State state, Matrix<bool> const &grid);
std::uint64_t
lowest_score(Location start, Location finish, Matrix<bool> const &grid);
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

  fmt::println("{}", lowest_score(lines));
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
        "#.......#....E#"sv,
        "#.#.###.#.###.#"sv,
        "#.....#.#...#.#"sv,
        "#.###.#####.#.#"sv,
        "#.#.#.......#.#"sv,
        "#.#.#####.###.#"sv,
        "#...........#.#"sv,
        "###.#.#####.#.#"sv,
        "#...#.....#.#.#"sv,
        "#.#.#.###.#.#.#"sv,
        "#.....#...#.#.#"sv,
        "#.###.#.#.#.#.#"sv,
        "#S..#.....#...#"sv,
        "###############"sv,
    };
    ASSERT(lowest_score(lines) == 7036);
  }
  {
    auto const lines = std::array{
        "#################"sv,
        "#...#...#...#..E#"sv,
        "#.#.#.#.#.#.#.#.#"sv,
        "#.#.#.#...#...#.#"sv,
        "#.#.#.#.###.#.#.#"sv,
        "#...#.#.#.....#.#"sv,
        "#.#.#.#.#.#####.#"sv,
        "#.#...#.#.#.....#"sv,
        "#.#.#####.#.###.#"sv,
        "#.#.#.......#...#"sv,
        "#.#.###.#####.###"sv,
        "#.#.#...#.....#.#"sv,
        "#.#.#.#####.###.#"sv,
        "#.#.#.........#.#"sv,
        "#.#.#.#########.#"sv,
        "#S#.............#"sv,
        "#################"sv,
    };
    ASSERT(lowest_score(lines) == 11048);
  }
}

std::uint64_t
lowest_score(std::ranges::range auto &&lines) {
  auto [start, finish, grid] = parse_map(lines);
  return lowest_score(start, finish, grid);
}

std::tuple<Location, Location, Matrix<bool>>
parse_map(std::ranges::range auto &&lines) {
  Location start{};
  Location finish{};
  Matrix<bool> grid(lines.size(), lines[0].size(), true);

  for (auto const [row, line] : std::views::enumerate(lines)) {
    for (auto const [col, ch] : std::views::enumerate(line)) {
      switch (ch) {
      case '#': {
        grid(size_t(row), size_t(col)) = false;
        break;
      }
      case 'S': {
        start = Location{size_t(row), size_t(col)};
        break;
      }
      case 'E': {
        finish = Location{size_t(row), size_t(col)};
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

std::pair<bool, State>
next_state(State state, Matrix<bool> const &grid) {
  switch (state.dir) {
  case Direction::UP: {
    if (!grid(state.loc.row - 1, state.loc.col)) {
      return {false, {}};
    }
    return {true, {{state.loc.row - 1, state.loc.col}, state.dir}};
  }
  case Direction::DOWN: {
    if (!grid(state.loc.row + 1, state.loc.col)) {
      return {false, {}};
    }
    return {true, {{state.loc.row + 1, state.loc.col}, state.dir}};
  }
  case Direction::LEFT: {
    if (!grid(state.loc.row, state.loc.col - 1)) {
      return {false, {}};
    }
    return {true, {{state.loc.row, state.loc.col - 1}, state.dir}};
  }
  case Direction::RIGHT: {
    if (!grid(state.loc.row, state.loc.col + 1)) {
      return {false, {}};
    }
    return {true, {{state.loc.row, state.loc.col + 1}, state.dir}};
  }
  default:
    UNREACHABLE(state.dir);
  }
}

std::uint64_t
lowest_score(Location start, Location finish, Matrix<bool> const &grid) {
  std::unordered_map<State, std::uint64_t> state_cost;
  state_cost[{start, Direction::RIGHT}] = 0;

  std::vector<State> to_explore;
  to_explore.emplace_back(start, Direction::RIGHT);

  while (!to_explore.empty()) {
    State state = to_explore.back();
    to_explore.pop_back();

    auto [success, nstate] = next_state(state, grid);
    if (success) {
      auto find_it = state_cost.find(nstate);
      auto next_cost = state_cost[state] + 1;
      if (find_it == state_cost.end() || find_it->second > next_cost) {
        state_cost[nstate] = state_cost[state] + 1;
        to_explore.emplace_back(nstate);
      }
    }

    nstate = state;
    for (std::uint8_t i = 0; i < 3; ++i) {
      nstate.dir = rotate_ccw(nstate.dir);
      auto find_it = state_cost.find(nstate);
      auto next_cost = state_cost[state] + (i == 0 || i == 2 ? 1000 : 2000);
      if (find_it == state_cost.end() || find_it->second > next_cost) {
        state_cost[nstate] = next_cost;
        to_explore.emplace_back(nstate);
      }
    }
  }

  auto min_score = std::numeric_limits<std::uint64_t>::max();
  for (Direction dir :
       {Direction::UP, Direction::DOWN, Direction::LEFT, Direction::RIGHT}) {
    auto it = state_cost.find({finish, dir});
    if (it != state_cost.end() && it->second < min_score) {
      min_score = it->second;
    }
  }
  return min_score;
}

void
print_grid(Matrix<bool> const &grid) {
  for (std::size_t row{}; row < grid.rows(); ++row) {
    for (std::size_t col{}; col < grid.cols(); ++col) {
      fmt::print("{}", grid(row, col) ? ' ' : '#');
    }
    fmt::println("");
  }
}
} // namespace
