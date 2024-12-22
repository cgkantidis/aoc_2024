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
#include <unordered_map>
#include <utility>

#include "matrix.hpp"

struct Location
{
  std::size_t row;
  std::size_t col;
};

enum Direction : unsigned
{
  UP,
  DOWN,
  LEFT,
  RIGHT,
};

auto constexpr ALL_DIRECTIONS = {Direction::UP,
                                 Direction::DOWN,
                                 Direction::LEFT,
                                 Direction::RIGHT};

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
rotate_cw(Direction dir) {
  switch (dir) {
  case Direction::UP:
    return Direction::RIGHT;
  case Direction::RIGHT:
    return Direction::DOWN;
  case Direction::DOWN:
    return Direction::LEFT;
  case Direction::LEFT:
    return Direction::UP;
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
operator==(Location const &lhs, Location const &rhs) {
  return lhs.row == rhs.row && lhs.col == rhs.col;
}
bool
operator==(State const &lhs, State const &rhs) {
  return lhs.loc == rhs.loc && lhs.dir == rhs.dir;
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
num_seats(std::ranges::range auto &&lines);
std::tuple<Location, Location, Matrix<char>>
parse_map(std::ranges::range auto &&lines);
std::pair<bool, State>
next_state(State state, Matrix<char> const &grid);
std::unordered_map<State, std::uint64_t>
get_state_cost(Location start, Matrix<char> &grid);
std::uint64_t
get_num_seats(Location finish,
              std::unordered_map<State, std::uint64_t> const &state_cost,
              Matrix<char> &grid);
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

  fmt::println("{}", num_seats(lines));
  return 0;
}

namespace
{
void
tests() {
  using namespace std::literals::string_view_literals;
  {
    auto const lines = std::array{
        "#######"sv,
        "##...E#"sv,
        "##.#.##"sv,
        "#S...##"sv,
        "#######"sv,
    };
    ASSERT(num_seats(lines) == 10);
  }
  {
    auto const lines = std::array{
        "######"sv,
        "#...E#"sv,
        "#....#"sv,
        "#S...#"sv,
        "######"sv,
    };
    ASSERT(num_seats(lines) == 6);
  }
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
    ASSERT(num_seats(lines) == 45);
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
    ASSERT(num_seats(lines) == 64);
  }
}

std::uint64_t
num_seats(std::ranges::range auto &&lines) {
  auto [start, finish, grid] = parse_map(lines);
  grid.print();
  auto state_cost = get_state_cost(start, grid);
  return get_num_seats(finish, state_cost, grid);
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

std::pair<bool, State>
next_state(State state, Matrix<char> const &grid) {
  switch (state.dir) {
  case Direction::UP: {
    if (grid(state.loc.row - 1, state.loc.col) == '#') {
      return {false, {}};
    }
    return {true, {{state.loc.row - 1, state.loc.col}, state.dir}};
  }
  case Direction::DOWN: {
    if (grid(state.loc.row + 1, state.loc.col) == '#') {
      return {false, {}};
    }
    return {true, {{state.loc.row + 1, state.loc.col}, state.dir}};
  }
  case Direction::LEFT: {
    if (grid(state.loc.row, state.loc.col - 1) == '#') {
      return {false, {}};
    }
    return {true, {{state.loc.row, state.loc.col - 1}, state.dir}};
  }
  case Direction::RIGHT: {
    if (grid(state.loc.row, state.loc.col + 1) == '#') {
      return {false, {}};
    }
    return {true, {{state.loc.row, state.loc.col + 1}, state.dir}};
  }
  default:
    UNREACHABLE(state.dir);
  }
}

std::unordered_map<State, std::uint64_t>
get_state_cost(Location start, Matrix<char> &grid) {
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
      if (find_it == state_cost.end() || find_it->second >= next_cost) {
        state_cost[nstate] = state_cost[state] + 1;
        to_explore.emplace_back(nstate);
      }
    }

    {
      nstate = state;
      nstate.dir = rotate_cw(nstate.dir);
      auto find_it = state_cost.find(nstate);
      auto next_cost = state_cost[state] + 1000;
      if (find_it == state_cost.end() || find_it->second >= next_cost) {
        state_cost[nstate] = next_cost;
        to_explore.emplace_back(nstate);
      }
    }
    {
      nstate = state;
      nstate.dir = rotate_ccw(nstate.dir);
      auto find_it = state_cost.find(nstate);
      auto next_cost = state_cost[state] + 1000;
      if (find_it == state_cost.end() || find_it->second >= next_cost) {
        state_cost[nstate] = next_cost;
        to_explore.emplace_back(nstate);
      }
    }
  }
  return state_cost;
}

std::uint64_t
get_num_seats(Location finish,
              std::unordered_map<State, std::uint64_t> const &state_cost,
              Matrix<char> &grid) {

  auto min_cost = std::numeric_limits<std::uint64_t>::max();
  for (auto dir : ALL_DIRECTIONS) {
    State s{finish, dir};
    auto s_it = state_cost.find(s);
    if (s_it != state_cost.end() && s_it->second < min_cost) {
      min_cost = s_it->second;
    }
  }

  std::deque<State> to_explore;
  for (auto dir : ALL_DIRECTIONS) {
    State s{finish, dir};
    auto s_it = state_cost.find(s);
    if (s_it != state_cost.end() && s_it->second == min_cost) {
      to_explore.push_back(s);
    }
  }

  while (!to_explore.empty()) {
    State s = to_explore.front();
    to_explore.pop_front();
    grid(s.loc.row, s.loc.col) = 'O';

    for (auto dir : ALL_DIRECTIONS) {
      State s2{};
      switch (s.dir) {
      case Direction::UP: {
        s2 = {{s.loc.row + 1, s.loc.col}, dir};
        break;
      }
      case Direction::DOWN: {
        s2 = {{s.loc.row - 1, s.loc.col}, dir};
        break;
      }
      case Direction::LEFT: {
        s2 = {{s.loc.row, s.loc.col + 1}, dir};
        break;
      }
      case Direction::RIGHT: {
        s2 = {{s.loc.row, s.loc.col - 1}, dir};
        break;
      }
      }

      auto s2_it = state_cost.find(s2);
      if (s2_it == state_cost.end()) {
        continue;
      }

      std::uint64_t expected_cost = s2_it->second + 1;
      if (dir != s.dir) {
        expected_cost += 1000;
      }

      auto s_it = state_cost.find(s);
      if (s_it->second == expected_cost) {
        to_explore.push_back(s2);
      }
    }
  }

  grid.print();

  std::uint64_t seats{};
  for (std::size_t row{}; row < grid.rows(); ++row) {
    for (std::size_t col{}; col < grid.cols(); ++col) {
      if (grid(row, col) == 'O') {
        ++seats;
      }
    }
  }
  return seats;
}
} // namespace
