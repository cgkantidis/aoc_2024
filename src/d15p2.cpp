#include <array>
#include <cstdint>
#include <fmt/core.h>
#include <fstream>
#include <libassert/assert.hpp>
#include <ranges>
#include <scn/scan.h>
#include <string>
#include <string_view>
#include <utility>

#include "matrix.hpp"

struct Robot
{
  std::uint64_t x;
  std::uint64_t y;
};

struct Item
{
  bool is_block;
  std::uint64_t x;
  std::uint64_t y;

  [[nodiscard]] bool
  is_above(Item const &other) const {
    return y + 1 == other.y
           && (x == other.x || x + 1 == other.x || x == other.x + 1);
  }
  [[nodiscard]] bool
  is_below(Item const &other) const {
    return y == other.y + 1
           && (x == other.x || x + 1 == other.x || x == other.x + 1);
  }
  [[nodiscard]] bool
  is_left_of(Item const &other) const {
    return y == other.y && x + 2 == other.x;
  }
  [[nodiscard]] bool
  is_right_of(Item const &other) const {
    return y == other.y && x == other.x + 2;
  }

  [[nodiscard]] bool
  is_above(Robot const &robot) const {
    return y + 1 == robot.y && (x == robot.x || x + 1 == robot.x);
  }
  [[nodiscard]] bool
  is_below(Robot const &robot) const {
    return y == robot.y + 1 && (x == robot.x || x + 1 == robot.x);
  }
  [[nodiscard]] bool
  is_left_of(Robot const &robot) const {
    return y == robot.y && x + 2 == robot.x;
  }
  [[nodiscard]] bool
  is_right_of(Robot const &robot) const {
    return y == robot.y && x == robot.x + 1;
  }
};

enum class Move
{
  UP,
  DOWN,
  LEFT,
  RIGHT,
};

namespace
{
void
tests();
std::uint64_t
sum_of_gps_coord(std::ranges::range auto &&lines);
std::tuple<Robot, std::vector<Move>, std::vector<Item>>
parse_robot_moves_items(std::ranges::range auto &&lines);
void
perform_movements(Robot &robot,
                  std::vector<Move> const &moves,
                  std::vector<Item> &items);
void
move_up(Robot &robot, std::vector<Item> &items);
void
move_down(Robot &robot, std::vector<Item> &items);
void
move_left(Robot &robot, std::vector<Item> &items);
void
move_right(Robot &robot, std::vector<Item> &items);
std::uint64_t
compute_sum_of_gps_coord(std::vector<Item> const &items);
void
print_grid(std::vector<Item> const &items, Robot const &robot);
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

  fmt::println("{}", sum_of_gps_coord(lines));
  return 0;
}

namespace
{
void
tests() {
  {
    Item it1{false, 6, 3};
    Item it2{false, 8, 3};
    Item it3{false, 6, 4};
    Item it4{false, 7, 4};
    ASSERT(it3.is_below(it1));
    ASSERT(!it3.is_below(it2));
    ASSERT(it4.is_below(it1));
    ASSERT(it4.is_below(it2));
  }

  using namespace std::literals::string_view_literals;
  {
    auto const lines = std::array{
        "#######"sv,
        "#...#.#"sv,
        "#.....#"sv,
        "#..OO@#"sv,
        "#..O..#"sv,
        "#.....#"sv,
        "#######"sv,
        ""sv,
        "<vv<<^^<<^^"sv,
    };
    ASSERT(sum_of_gps_coord(lines) == 618);
  }
  {
    auto const lines = std::array{
        "########"sv,
        "#..O.O.#"sv,
        "##@.O..#"sv,
        "#...O..#"sv,
        "#.#.O..#"sv,
        "#...O..#"sv,
        "#......#"sv,
        "########"sv,
        ""sv,
        "<^^>>>vv<v>>v<<"sv,
    };
    ASSERT(sum_of_gps_coord(lines) == 1751);
  }
  {
    auto const lines = std::array{
        "##########"sv,
        "#..O..O.O#"sv,
        "#......O.#"sv,
        "#.OO..O.O#"sv,
        "#..O@..O.#"sv,
        "#O#..O...#"sv,
        "#O..O..O.#"sv,
        "#.OO.O.OO#"sv,
        "#....O...#"sv,
        "##########"sv,
        ""sv,
        "<vv>^<v^>v>^vv^v>v<>v^v<v<^vv<<<^><<><>>v<vvv<>^v^>^<<<><<v<<<v^vv^v>^"sv,
        "vvv<<^>^v^^><<>>><>^<<><^vv^^<>vvv<>><^^v>^>vv<>v<<<<v<^v>^<^^>>>^<v<v"sv,
        "><>vv>v^v^<>><>>>><^^>vv>v<^^^>>v^v^<^^>v^^>v^<^v>v<>>v^v^<v>v^^<^^vv<"sv,
        "<<v<^>>^^^^>>>v^<>vvv^><v<<<>^^^vv^<vvv>^>v<^^^^v<>^>vvvv><>>v^<<^^^^^"sv,
        "^><^><>>><>^^<<^^v>>><^<v>^<vv>>v>>>^v><>^v><<<<v>>v<v<v>vvv>^<><<>^><"sv,
        "^>><>^v<><^vvv<^^<><v<<<<<><^v<<<><<<^^<v<^^^><^>>^<v^><<<^>>^v<v^v<v^"sv,
        ">^>>^v>vv>^<<^v<>><<><<v<<v><>v<^vv<<<>^^v^>^^>>><<^v>>v^v><^^>>^<>vv^"sv,
        "<><^^>^^^<><vvvvv^v<v<<>^v<v>v<<^><<><<><<<^^<<<^<<>><<><^^^>^^<>^>v<>"sv,
        "^^>vv<^v^v<vv>^<><v<^v>^^^>>>^^vvv^>vvv<>>>^<^>>>>>^<<^v>^vvv<>^<><<v>"sv,
        "v^^>>><<^^<>>^v^<v^vv<>v^<<>^<^v^v><^<<<><<^<v><v<>vv>>v><v^<vv<>v^<<^"sv,
    };
    ASSERT(sum_of_gps_coord(lines) == 9021);
  }
}

std::uint64_t
sum_of_gps_coord(std::ranges::range auto &&lines) {
  auto [robot, moves, items] = parse_robot_moves_items(lines);
  perform_movements(robot, moves, items);
  return compute_sum_of_gps_coord(items);
}

std::tuple<Robot, std::vector<Move>, std::vector<Item>>
parse_robot_moves_items(std::ranges::range auto &&lines) {
  // find the number of rows in the grid
  auto const rows = static_cast<std::size_t>(
      std::distance(lines.begin(), std::ranges::find(lines, "")));
  auto const cols = lines[0].size();

  std::vector<Item> items;
  Robot robot{};
  for (std::size_t row{}; row < rows; ++row) {
    for (std::size_t col{}; col < cols; ++col) {
      auto ch = lines[row][col];
      switch (ch) {
      case '#': {
        items.emplace_back(true, col * 2, row);
        break;
      }
      case 'O': {
        items.emplace_back(false, col * 2, row);
        break;
      }
      case '@': {
        robot = Robot{col * 2, row};
        break;
      }
      case '.': {
        break;
      }
      default: {
        UNREACHABLE(ch);
      }
      }
    }
  }

  std::vector<Move> moves;
  for (auto const &line : lines | std::views::drop(rows + 1)) {
    std::ranges::transform(line, std::back_inserter(moves), [](char ch) {
      switch (ch) {
      case '^': {
        return Move::UP;
      }
      case 'v': {
        return Move::DOWN;
      }
      case '<': {
        return Move::LEFT;
      }
      case '>': {
        return Move::RIGHT;
      }
      default: {
        UNREACHABLE(ch);
      }
      }
    });
  }
  return {robot, moves, items};
}

void
perform_movements(Robot &robot,
                  std::vector<Move> const &moves,
                  std::vector<Item> &items) {
  for (Move const &move : moves) {
    switch (move) {
    case Move::UP: {
      move_up(robot, items);
      break;
    }
    case Move::DOWN: {
      move_down(robot, items);
      break;
    }
    case Move::LEFT: {
      move_left(robot, items);
      break;
    }
    case Move::RIGHT: {
      move_right(robot, items);
      break;
    }
    }
  }
  print_grid(items, robot);
}

void
move_up(Robot &robot, std::vector<Item> &items) {
  auto find_it = std::ranges::find_if(items, [&robot](Item const &item) {
    return item.is_above(robot);
  });
  if (find_it == items.end()) {
    // nothing was above the robot
    --robot.y;
    return;
  }

  std::vector<Item *> to_move;
  std::vector<Item *> to_explore;

  to_explore.emplace_back(&*find_it);
  while (!to_explore.empty()) {
    Item &exp_item = *to_explore.back();
    to_explore.pop_back();
    if (exp_item.is_block) {
      // no movement is possible
      return;
    }

    for (Item &item : items) {
      if (item.is_above(exp_item)) {
        to_explore.emplace_back(&item);
      }
    }
    to_move.emplace_back(&exp_item);
  }

  for (Item *item : to_move) {
    --item->y;
  }
  --robot.y;
}
void
move_down(Robot &robot, std::vector<Item> &items) {
  auto find_it = std::ranges::find_if(items, [&robot](Item const &item) {
    return item.is_below(robot);
  });
  if (find_it == items.end()) {
    // nothing was above the robot
    ++robot.y;
    return;
  }

  std::vector<Item *> to_move;
  std::vector<Item *> to_explore;

  to_explore.emplace_back(&*find_it);
  while (!to_explore.empty()) {
    Item &exp_item = *to_explore.back();
    to_explore.pop_back();
    if (exp_item.is_block) {
      // no movement is possible
      return;
    }

    for (Item &item : items) {
      if (item.is_below(exp_item)) {
        to_explore.emplace_back(&item);
      }
    }
    to_move.emplace_back(&exp_item);
  }

  for (Item *item : to_move) {
    ++item->y;
  }
  ++robot.y;
}
void
move_left(Robot &robot, std::vector<Item> &items) {
  auto find_it = std::ranges::find_if(items, [&robot](Item const &item) {
    return item.is_left_of(robot);
  });
  if (find_it == items.end()) {
    // nothing was above the robot
    --robot.x;
    return;
  }

  std::vector<Item *> to_move;
  std::vector<Item *> to_explore;

  to_explore.emplace_back(&*find_it);
  while (!to_explore.empty()) {
    Item &exp_item = *to_explore.back();
    to_explore.pop_back();
    if (exp_item.is_block) {
      // no movement is possible
      return;
    }

    auto find_item = std::ranges::find_if(items, [&exp_item](Item const &item) {
      return item.is_left_of(exp_item);
    });
    if (find_item != items.end()) {
      to_explore.emplace_back(&*find_item);
    }
    to_move.emplace_back(&exp_item);
  }

  for (Item *item : to_move) {
    --item->x;
  }
  --robot.x;
}
void
move_right(Robot &robot, std::vector<Item> &items) {
  auto find_it = std::ranges::find_if(items, [&robot](Item const &item) {
    return item.is_right_of(robot);
  });
  if (find_it == items.end()) {
    // nothing was above the robot
    ++robot.x;
    return;
  }

  std::vector<Item *> to_move;
  std::vector<Item *> to_explore;

  to_explore.emplace_back(&*find_it);
  while (!to_explore.empty()) {
    Item &exp_item = *to_explore.back();
    to_explore.pop_back();
    if (exp_item.is_block) {
      // no movement is possible
      return;
    }

    auto find_item = std::ranges::find_if(items, [&exp_item](Item const &item) {
      return item.is_right_of(exp_item);
    });
    if (find_item != items.end()) {
      to_explore.emplace_back(&*find_item);
    }
    to_move.emplace_back(&exp_item);
  }

  for (Item *item : to_move) {
    ++item->x;
  }
  ++robot.x;
}

std::uint64_t
compute_sum_of_gps_coord(std::vector<Item> const &items) {
  return std::ranges::fold_left(items, 0, [](std::uint64_t prev, Item const &item) {
    return prev + (item.is_block ? 0 : item.y * 100 + item.x);
  });
}

void
print_grid(std::vector<Item> const &items, Robot const &robot) {
  std::size_t max_row =
      std::ranges::max_element(items, [](Item const &lhs, Item const &rhs) {
        return lhs.y < rhs.y;
      })->y;
  std::size_t max_col =
      std::ranges::max_element(items, [](Item const &lhs, Item const &rhs) {
        return lhs.x < rhs.x;
      })->x;

  Matrix<char> grid(max_row + 1, max_col + 2, ' ');
  for (Item const &item : items) {
    if (item.is_block) {
      grid(item.y, item.x) = '#';
      grid(item.y, item.x + 1) = '#';
    } else {
      grid(item.y, item.x) = '[';
      grid(item.y, item.x + 1) = ']';
    }
  }

  grid(robot.y, robot.x) = '@';

  grid.print();
}

} // namespace
