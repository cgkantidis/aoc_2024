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
bool
is_safe_int(std::ranges::input_range auto &&range);
bool
is_safe(std::ranges::input_range auto &&range);
std::uint64_t
get_num_safe(std::ranges::range auto &&lines);
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
    ASSERT(get_num_safe(lines) == 2);
  }
}

std::uint64_t
get_num_safe(std::ranges::range auto &&lines) {
  std::size_t total{};
  for (auto const &line : lines) {
    auto tokens = split(line);
    if (is_safe(tokens | std::views::transform(str_to_int<std::uint64_t>))) {
      ++total;
    }
  }
  return total;
}

bool
is_safe_int(std::ranges::input_range auto &&range) {
  return std::ranges::all_of(range | std::views::slide(2), [](auto const &window) {
    return window[0] < window[1] && window[1] - window[0] < 4;
  });
}

bool
is_safe(std::ranges::input_range auto &&range) {
  return is_safe_int(range) || is_safe_int(std::ranges::reverse_view(range));
}
} // namespace
