#include <array> // std::array
#include <cstdint> // std::uint64_t
#include <fmt/core.h> // fmt::print
#include <fmt/ranges.h> // fmt::print
#include <fstream> // std::ifstream
#include <libassert/assert.hpp> // ASSERT
#include <ranges> // std::views::enumerate
#include <string> // std::string
#include <unordered_map>
#include <unordered_set>
#include <vector> // std::vector

#include "BS_thread_pool.hpp"
#include "utility.hpp" // str_to_int

using ulong = std::uint64_t;
using State = std::array<long, 4>;

/// we need the operator==() to resolve hash collisions
bool
operator==(State const &lhs, State const &rhs) {
  return std::ranges::all_of(std::views::zip(lhs, rhs), [](auto const &p) {
    return std::get<0>(p) == std::get<1>(p);
  });
}

/// custom specialization of std::hash injected in namespace std
template <>
struct std::hash<State>
{
  std::size_t
  operator()(State const &state) const noexcept {
    std::size_t h1 = std::hash<long>{}(state[0]);
    std::size_t h2 = std::hash<long>{}(state[1]);
    std::size_t h3 = std::hash<long>{}(state[2]);
    std::size_t h4 = std::hash<long>{}(state[3]);

    std::size_t ret_val = 0;
    hash_combine(ret_val, h1, h2, h3, h4);
    return ret_val;
  }
};

static constexpr ulong NUM_ITERATIONS{2000};

namespace
{
void
tests();
ulong
get_max_price(std::ranges::range auto &&lines, ulong iterations);
std::unordered_map<State, ulong>
get_secret_number(ulong seed, ulong iterations);
constexpr ulong
get_price(ulong seed) {
  return seed % 10;
}
void
shift_and_append(std::array<long, 4> &prev, long diff);
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

  fmt::println("{}", get_max_price(lines, NUM_ITERATIONS));
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
        "2"sv,
        "3"sv,
        "2024"sv,
    };
    ASSERT(get_max_price(lines, NUM_ITERATIONS) == 23);
  }
}

ulong
get_max_price(std::ranges::range auto &&lines, ulong iterations) {
  BS::thread_pool pool;
  std::vector<std::future<std::unordered_map<State, ulong>>> futures;
  futures.reserve(lines.size());

  std::ranges::transform(lines,
                         std::back_inserter(futures),
                         [&iterations, &pool](auto const &line) {
                           return pool.submit_task([&iterations, &line] {
                             return get_secret_number(str_to_int<ulong>(line),
                                                      iterations);
                           });
                         });

  std::vector<std::unordered_map<State, ulong>> state_price_maps;
  state_price_maps.reserve(lines.size());
  std::ranges::transform(futures,
                         std::back_inserter(state_price_maps),
                         [](auto &fut) { return fut.get(); });

  std::unordered_set<State> all_states;
  for (auto const &state_price_map : state_price_maps) {
    for (auto const &state_price : state_price_map) {
      all_states.insert(state_price.first);
    }
  }

  ulong max_price = 0;
  for (auto const &state : all_states) {
    ulong total_price = std::ranges::fold_left(
        state_price_maps,
        0ULL,
        [&state](ulong prev, auto const &state_price_map) {
          auto find_it = state_price_map.find(state);
          if (find_it != state_price_map.end()) {
            return prev + find_it->second;
          }
          return prev;
        });
    if (total_price > max_price) {
      max_price = total_price;
    }
  }
  return max_price;
}

std::unordered_map<State, ulong>
get_secret_number(ulong seed, ulong iterations) {
  static constexpr ulong shift1{6ULL}; // multiply by 64
  static constexpr ulong shift2{5ULL}; // divide by 32
  static constexpr ulong shift3{11ULL}; // multiply by 2048
  static constexpr ulong prune_bits{(1ULL << 24ULL) - 1}; // modulo 16777216

  std::unordered_map<State, ulong> diffs_to_price;

  std::array<long, 4> diffs{};
  ulong prev_price = get_price(seed);
  for (ulong iter{}; iter < iterations; ++iter) {
    seed = ((seed << shift1) ^ seed) & prune_bits;
    seed = ((seed >> shift2) ^ seed) & prune_bits;
    seed = ((seed << shift3) ^ seed) & prune_bits;

    ulong price = get_price(seed);
    long diff = long(price) - long(prev_price);
    prev_price = price;
    shift_and_append(diffs, diff);

    if (iter > 2 && diffs_to_price.find(diffs) == diffs_to_price.end()) {
      diffs_to_price[diffs] = price;
    }
  }
  return diffs_to_price;
}

void
shift_and_append(std::array<long, 4> &prev, long diff) {
  prev[0] = prev[1];
  prev[1] = prev[2];
  prev[2] = prev[3];
  prev[3] = diff;
}
} // namespace
