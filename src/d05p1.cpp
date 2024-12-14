#include "utility.hpp"
#include <algorithm>
#include <fmt/core.h>
#include <fmt/ranges.h>
#include <fstream>
#include <map>
#include <ranges>
#include <set>
#include <string>
#include <string_view>

#include <libassert/assert.hpp>

using namespace std::literals::string_view_literals;

bool
is_page_valid(std::vector<std::uint64_t> const &page,
              std::map<std::uint64_t, std::set<uint64_t>> &order_rules) {
  for (auto it_i = page.begin(); it_i != page.end(); ++it_i) {
    for (auto it_j = std::next(it_i); it_j != page.end(); ++it_j) {
      auto find_it = order_rules.find(*it_j);
      if (find_it != order_rules.end()) {
        if (find_it->second.contains(*it_i)) {
          return false;
        }
      }
    }
  }
  return true;
}

std::uint64_t
get_page_value(std::vector<std::uint64_t> const &page,
               std::map<std::uint64_t, std::set<uint64_t>> &order_rules) {
  return is_page_valid(page, order_rules) ? page[page.size() / 2] : 0;
}

std::pair<std::map<std::uint64_t, std::set<uint64_t>>,
          std::vector<std::vector<std::uint64_t>>>
scan_order_rules_and_pages(std::ranges::range auto &&lines) {
  auto line_it = lines.begin();
  auto line_end = lines.end();

  std::map<std::uint64_t, std::set<uint64_t>> order_rules;
  while (line_it != line_end) {
    if (line_it->empty()) {
      ++line_it;
      break;
    }
    auto tokens = std::views::transform(split(*line_it, "|"), str_to_int<std::uint64_t>);
    auto find_it = order_rules.find(tokens[0]);
    if (find_it == order_rules.end()) {
      order_rules.emplace(tokens[0], std::set{tokens[1]});
    } else {
      find_it->second.insert(tokens[1]);
    }
    ++line_it;
  }

  std::vector<std::vector<std::uint64_t>> pages;
  while (line_it != line_end) {
    auto tokens = std::ranges::to<std::vector>(
        std::views::transform(split(*line_it, ","), str_to_int<std::uint64_t>));
    pages.emplace_back(std::move(tokens));
    ++line_it;
  }

  return {order_rules, pages};
}

std::uint64_t
sum_middle_of_valid(std::ranges::range auto &&lines) {
  auto [order_rules, pages] = scan_order_rules_and_pages(lines);
  return std::ranges::fold_left(
      pages,
      0ULL,
      [&order_rules](std::uint64_t const &prev,
                     std::vector<std::uint64_t> const &page) {
        return prev + get_page_value(page, order_rules);
      });
}

void
tests() {
  auto lines = std::vector{{"47|53"sv,          "97|13"sv,
                            "97|61"sv,          "97|47"sv,
                            "75|29"sv,          "61|13"sv,
                            "75|53"sv,          "29|13"sv,
                            "97|29"sv,          "53|29"sv,
                            "61|53"sv,          "97|53"sv,
                            "61|29"sv,          "47|13"sv,
                            "75|47"sv,          "97|75"sv,
                            "47|61"sv,          "75|61"sv,
                            "47|29"sv,          "75|13"sv,
                            "53|13"sv,          ""sv,
                            "75,47,61,53,29"sv, "97,61,53,29,13"sv,
                            "75,29,13"sv,       "75,97,47,61,53"sv,
                            "61,13,29"sv,       "97,13,75,29,47"sv}};

  ASSERT(sum_middle_of_valid(lines) == 143);
}

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

  fmt::println("{}", sum_middle_of_valid(lines));
  return 0;
}
