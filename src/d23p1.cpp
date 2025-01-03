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
using adj_list_t = std::unordered_map<std::string, std::vector<std::string>>;
// struct path_t {
//   std::vector<std::string> nodes;
// };

using path_t = std::vector<std::string>;

/// we need the operator==() to resolve hash collisions
template <>
struct std::equal_to<path_t>
{
  bool
  operator()(path_t const &lhs, path_t const &rhs) const {
    auto lhs_cpy = lhs;
    auto rhs_cpy = rhs;
    std::ranges::sort(lhs_cpy);
    std::ranges::sort(rhs_cpy);
    return std::ranges::all_of(
        std::views::zip(lhs_cpy, rhs_cpy),
        [](auto const &p) { return std::get<0>(p) == std::get<1>(p); });
  }
};

/// custom specialization of std::hash injected in namespace std
template <>
struct std::hash<path_t>
{
  std::size_t
  operator()(path_t const &path) const noexcept {
    auto h = std::hash<std::string>{}(path[0])
             ^ std::hash<std::string>{}(path[1])
             ^ std::hash<std::string>{}(path[2]);
    return h;
  }
};
using path_set_t = std::unordered_set<path_t>;

namespace
{
void
tests();
ulong
get_num_cycles_of_length_three_with_t(std::ranges::range auto &&lines);
adj_list_t
parse_adj_list(std::ranges::range auto &&lines);
path_set_t
all_cycles_of_length_three(adj_list_t const &adj_list);
path_set_t
all_cycles_of_length_three(std::string const &src, adj_list_t const &adj_list);
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

  fmt::println("{}", get_num_cycles_of_length_three_with_t(lines));
  return 0;
}

namespace
{
void
tests() {
  using namespace std::literals::string_view_literals;
  {
    auto const lines = std::array{
        "kh-tc"sv, "qp-kh"sv, "de-cg"sv, "ka-co"sv, "yn-aq"sv, "qp-ub"sv,
        "cg-tb"sv, "vc-aq"sv, "tb-ka"sv, "wh-tc"sv, "yn-cg"sv, "kh-ub"sv,
        "ta-co"sv, "de-co"sv, "tc-td"sv, "tb-wq"sv, "wh-td"sv, "ta-ka"sv,
        "td-qp"sv, "aq-cg"sv, "wq-ub"sv, "ub-vc"sv, "de-ta"sv, "wq-aq"sv,
        "wq-vc"sv, "wh-yn"sv, "ka-de"sv, "kh-ta"sv, "co-tc"sv, "wh-qp"sv,
        "tb-vc"sv, "td-yn"sv,
    };
    ASSERT(get_num_cycles_of_length_three_with_t(lines) == 7);
  }
}

ulong
get_num_cycles_of_length_three_with_t(std::ranges::range auto &&lines) {
  auto adj_list = parse_adj_list(lines);
  auto all_cycles = all_cycles_of_length_three(adj_list);
  return std::ranges::count_if(all_cycles, [](auto const &cycle) {
    return std::ranges::any_of(cycle,
                               [](auto const &node) { return node[0] == 't'; });
  });
}

adj_list_t
parse_adj_list(std::ranges::range auto &&lines) {
  adj_list_t adj_list;
  auto add_to_map = [&adj_list](std::string_view src, std::string_view dst) {
    auto [it, success] =
        adj_list.try_emplace(std::string(src),
                             std::vector<std::string>{std::string(dst)});
    if (!success) {
      it->second.emplace_back(dst);
    }
  };
  for (auto const &line : lines) {
    auto src_dst = split(line, "-");
    add_to_map(src_dst[0], src_dst[1]);
    add_to_map(src_dst[1], src_dst[0]);
  }
  return adj_list;
}

path_set_t
all_cycles_of_length_three(adj_list_t const &adj_list) {
  path_set_t all_cycles;
  for (auto const &[v, neighbors] : adj_list) {
    all_cycles.merge(all_cycles_of_length_three(v, adj_list));
  }
  return all_cycles;
}

path_set_t
all_cycles_of_length_three(std::string const &src, adj_list_t const &adj_list) {
  path_set_t all_cycles;

  std::queue<path_t> to_visit;
  to_visit.emplace(path_t{{src}});

  while (!to_visit.empty()) {
    auto path = to_visit.front();
    to_visit.pop();

    for (auto const &neighbor : adj_list.find(path.back())->second) {
      if (neighbor == path.back()) {
        // the vertex we just came from
        continue;
      }

      if (path.size() < 3) {
        auto npath = path;
        npath.emplace_back(neighbor);
        to_visit.push(npath);
      } else if (path.size() == 3) {
        if (neighbor == path.front()) {
          // we found a cycle
          all_cycles.insert(path);
        }
      }
    }
  }
  return all_cycles;
}

} // namespace
