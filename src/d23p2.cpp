#include <array> // std::array
#include <cstdint> // std::uint64_t
#include <fmt/core.h> // fmt::print
#include <fmt/ranges.h> // fmt::print
#include <fstream> // std::ifstream
#include <libassert/assert.hpp> // ASSERT
#include <ranges> // std::views::enumerate
#include <sstream>
#include <stack>
#include <string> // std::string
#include <unordered_map>
#include <unordered_set>
#include <vector> // std::vector

#include "BS_thread_pool.hpp"
#include "utility.hpp" // str_to_int

using ulong = std::uint64_t;
using clique_t = std::unordered_set<std::string>;
using adj_list_t = std::unordered_map<std::string, clique_t>;

namespace
{
void
tests();
std::string
get_largest_clique(std::ranges::range auto &&lines);
adj_list_t
parse_adj_list(std::ranges::range auto &&lines);
clique_t
find_largest_clique(adj_list_t const &adj_list);
clique_t
find_largest_clique(std::string const &src, adj_list_t const &adj_list);
bool
is_vertex_in_clique(clique_t const &path, std::string const &v);
bool
is_vertex_connected_to_all_vertices_in_path(clique_t const &path,
                                            adj_list_t const &adj_list,
                                            std::string const &src);
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

  fmt::println("{}", get_largest_clique(lines));
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
    ASSERT(get_largest_clique(lines) == "co,de,ka,ta");
  }
}

std::string
get_largest_clique(std::ranges::range auto &&lines) {
  auto adj_list = parse_adj_list(lines);
  auto largest_clique = find_largest_clique(adj_list);
  auto largest_clique_vec =
      std::ranges::to<std::vector<std::string>>(largest_clique);
  std::ranges::sort(largest_clique_vec);
  std::stringstream ss;
  bool is_first = true;
  for (auto const &v : largest_clique_vec) {
    if (!is_first) {
      ss << ',' << v;
    } else {
      ss << v;
      is_first = false;
    }
  }
  return ss.str();
}

adj_list_t
parse_adj_list(std::ranges::range auto &&lines) {
  adj_list_t adj_list;
  auto add_to_map = [&adj_list](std::string_view src, std::string_view dst) {
    auto [it, success] =
        adj_list.try_emplace(std::string(src),
                             clique_t{std::string(dst)});
    if (!success) {
      it->second.emplace(dst);
    }
  };
  for (auto const &line : lines) {
    auto src_dst = split(line, "-");
    add_to_map(src_dst[0], src_dst[1]);
    add_to_map(src_dst[1], src_dst[0]);
  }
  return adj_list;
}

clique_t
find_largest_clique(adj_list_t const &adj_list) {
  clique_t largest_clique{};
  for (auto const &[v, neighbors] : adj_list) {
    auto clique = find_largest_clique(v, adj_list);
    if (clique.size() > largest_clique.size()) {
      largest_clique = clique;
    }
  }
  return largest_clique;
}

clique_t
find_largest_clique(std::string const &src, adj_list_t const &adj_list) {
  clique_t largest_clique{{src}};

  std::stack<std::pair<std::string, clique_t>> to_visit;
  to_visit.emplace(src, clique_t{src});

  while (!to_visit.empty()) {
    auto [v, clique] = to_visit.top();
    to_visit.pop();

    for (auto const &neighbor : adj_list.find(v)->second) {
      if (is_vertex_in_clique(largest_clique, neighbor)) {
        // already in the path
        continue;
      }

      if (is_vertex_connected_to_all_vertices_in_path(clique,
                                                      adj_list,
                                                      neighbor)) {
        auto nclique = clique;
        nclique.emplace(neighbor);
        to_visit.emplace(neighbor, nclique);

        if (nclique.size() > largest_clique.size()) {
          largest_clique = nclique;
        }
      }
    }
  }
  return largest_clique;
}

bool
is_vertex_in_clique(clique_t const &clique, std::string const &v) {
  return std::ranges::find(clique, v) != clique.end();
}

bool
is_vertex_connected_to_all_vertices_in_path(clique_t const &clique,
                                            adj_list_t const &adj_list,
                                            std::string const &src) {
  return std::ranges::all_of(clique, [&adj_list, &src](std::string const &dst) {
    return is_vertex_in_clique(adj_list.find(dst)->second, src);
  });
}

} // namespace
