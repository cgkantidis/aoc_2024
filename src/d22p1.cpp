#include <array> // std::array
#include <cstdint> // std::uint64_t
#include <fmt/core.h> // fmt::print
#include <fmt/ranges.h> // fmt::print
#include <fstream> // std::ifstream
#include <libassert/assert.hpp> // ASSERT
#include <ranges> // std::views::enumerate
#include <string> // std::string
#include <vector> // std::vector

#include "BS_thread_pool.hpp"
#include "utility.hpp" // str_to_int

static constexpr std::uint64_t NUM_ITERATIONS{2000};

namespace
{
void
tests();
std::uint64_t
get_sum_of_secret_numbers(std::ranges::range auto &&lines,
                          std::uint64_t iterations);
std::uint64_t
get_secret_number(std::uint64_t seed, std::uint64_t iterations);
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

  fmt::println("{}", get_sum_of_secret_numbers(lines, NUM_ITERATIONS));
  return 0;
}

namespace
{
void
tests() {
  using namespace std::literals::string_view_literals;
  {
    auto const lines = std::array{
        "1"sv,
        "10"sv,
        "100"sv,
        "2024"sv,
    };
    ASSERT(get_sum_of_secret_numbers(lines, NUM_ITERATIONS) == 37327623);
  }
}

std::uint64_t
get_sum_of_secret_numbers(std::ranges::range auto &&lines,
                          std::uint64_t iterations) {
  BS::thread_pool pool;
  std::vector<std::future<std::uint64_t>> futures;
  futures.reserve(lines.size());

  std::ranges::transform(
      lines,
      std::back_inserter(futures),
      [&iterations, &pool](auto const &line) {
        return pool.submit_task([&iterations, &line] {
          return get_secret_number(str_to_int<std::uint64_t>(line), iterations);
        });
      });

  return std::ranges::fold_left(futures, 0ULL, [](std::uint64_t prev, auto &fut) {
    return prev + fut.get();
  });
}

std::uint64_t
get_secret_number(std::uint64_t seed, std::uint64_t iterations) {
  static constexpr std::uint64_t shift1{6ULL}; // multiply by 64
  static constexpr std::uint64_t shift2{5ULL}; // divide by 32
  static constexpr std::uint64_t shift3{11ULL}; // multiply by 2048
  static constexpr std::uint64_t prune_bits{(1ULL << 24ULL) - 1}; // modulo 16777216

  for (std::uint64_t iter{}; iter < iterations; ++iter) {
    seed = ((seed << shift1) ^ seed) & prune_bits;
    seed = ((seed >> shift2) ^ seed) & prune_bits;
    seed = ((seed << shift3) ^ seed) & prune_bits;
  }
  return seed;
}
} // namespace
