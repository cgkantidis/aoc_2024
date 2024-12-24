#include <algorithm> // std::ranges::all_of
#include <cstdint> // std::uint64_t
#include <fmt/core.h> // fmt::println
#include <fstream> // std::ifstream
#include <ranges> // std::ranges::range
#include <string> // std::string
#include <vector> // std::vector

#include "utility.hpp" // split

namespace
{
void
tests();
std::uint64_t
get_num_safe(std::ranges::range auto &&lines);
bool
are_increasing(std::size_t a, std::size_t b);
bool
is_safe_int(std::ranges::range auto &&levels);
bool
is_safe_int(std::ranges::range auto &&levels, std::size_t pivot);
bool
is_safe(std::ranges::range auto &&levels);
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

  fmt::println("{}", get_num_safe(lines));
  return 0;
}

namespace
{
void
tests() {
  using namespace std::literals::string_view_literals;
  {
    auto const lines = std::array{
        "7 6 4 2 1"sv,
        "1 2 7 8 9"sv,
        "9 7 6 2 1"sv,
        "1 3 2 4 5"sv,
        "8 6 4 4 1"sv,
        "1 3 6 7 9"sv,
    };
    ASSERT(get_num_safe(lines) == 4);
  }
}

std::uint64_t
get_num_safe(std::ranges::range auto &&lines) {
  std::size_t total{};
  for (auto const &line : lines) {
    std::vector<std::string_view> tokens = split(line);
    std::vector<std::uint64_t> levels;
    std::ranges::transform(tokens,
                           std::back_inserter(levels),
                           str_to_int<std::uint64_t>);
    if (is_safe(levels)) {
      ++total;
    }
  }
  return total;
}

bool
are_increasing(std::size_t a, std::size_t b) {
  return (a < b) && (b - a < 4);
}

bool
is_safe_int(std::ranges::range auto &&levels) {
  return std::ranges::all_of(std::views::slide(levels, 2), [](auto window) {
    return are_increasing(window[0], window[1]);
  });
}

bool
is_safe_int(std::ranges::range auto &&levels, std::size_t pivot) {
  auto left_end = std::next(levels.begin(), pivot);
  auto left = std::ranges::subrange(levels.begin(), left_end);
  auto right_begin = std::next(left_end, 1);
  auto right = std::ranges::subrange(right_begin, levels.end());

  if (left.empty()) {
    return is_safe_int(right);
  }
  if (right.empty()) {
    return is_safe_int(left);
  }
  return is_safe_int(left) && is_safe_int(right)
         && are_increasing(levels[pivot - 1], levels[pivot + 1]);
}

bool
is_safe(std::ranges::range auto &&levels) {
  for (std::size_t pivot = 0; pivot < levels.size(); ++pivot) {
    if (is_safe_int(levels, pivot)
        || is_safe_int(std::views::reverse(levels), pivot)) {
      return true;
    }
  }
  return false;
}
} // namespace
