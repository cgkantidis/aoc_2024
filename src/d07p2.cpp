#include "fmt/core.h"
#include "libassert/assert.hpp"
#include <algorithm>
#include <cstdint>
#include <fstream>
#include <ranges>
#include <string>
#include <string_view>
#include <utility>

#include "utility.hpp"

static void
tests();
static std::uint64_t
sum_valid_equation(std::ranges::range auto &&lines);
static std::pair<std::vector<std::uint64_t>,
                 std::vector<std::vector<std::uint64_t>>>
parse_totals_and_operands(std::ranges::range auto &&lines);
static bool
is_equation_valid(std::uint64_t const &target_total,
                  std::vector<std::uint64_t> const &operands);
static std::uint64_t
compute_total(std::vector<std::uint64_t> const &operands,
              std::vector<char> const &operators);
static bool
advance_operators(std::vector<char> &operators);

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

  fmt::println("{}", sum_valid_equation(lines));
  return 0;
}

void
tests() {
  using namespace std::literals::string_view_literals;
  auto lines = std::vector{{"190: 10 19"sv,
                            "3267: 81 40 27"sv,
                            "83: 17 5"sv,
                            "156: 15 6"sv,
                            "7290: 6 8 6 15"sv,
                            "161011: 16 10 13"sv,
                            "192: 17 8 14"sv,
                            "21037: 9 7 18 13"sv,
                            "292: 11 6 16 20"sv}};

  ASSERT(sum_valid_equation(lines) == 11387);
}

std::uint64_t
sum_valid_equation(std::ranges::range auto &&lines) {
  auto [totals, operands] = parse_totals_and_operands(lines);
  std::uint64_t total_valid{};
  for (auto const &[total, oper] : std::views::zip(totals, operands)) {
    if (is_equation_valid(total, oper)) {
      total_valid += total;
    }
  }
  return total_valid;
}

std::pair<std::vector<std::uint64_t>, std::vector<std::vector<std::uint64_t>>>
parse_totals_and_operands(std::ranges::range auto &&lines) {
  std::vector<std::uint64_t> totals;
  std::vector<std::vector<std::uint64_t>> operands;
  for (auto const &line : lines) {
    auto tokens = split(line, ": ");
    totals.emplace_back(str_to_int<std::uint64_t>(tokens[0]));
    std::vector<std::uint64_t> oper;
    std::ranges::transform(split(tokens[1], " "),
                           std::back_inserter(oper),
                           str_to_int<std::uint64_t>);
    operands.emplace_back(std::move(oper));
  }
  return {totals, operands};
}

bool
is_equation_valid(std::uint64_t const &target_total,
                  std::vector<std::uint64_t> const &operands) {
  std::vector<char> operators(operands.size(), '+'); // we don't use operators[0]
  if (!operators.empty()) {
    operators[0] = 'X';
  }

  while (true) {
    auto total = compute_total(operands, operators);
    if (total == target_total) {
      return true;
    }
    if (!advance_operators(operators)) {
      return false;
    }
  }

  ASSERT(false, "unreachable");
  std::unreachable();
}

std::uint64_t
compute_total(std::vector<std::uint64_t> const &operands,
              std::vector<char> const &operators) {
  std::uint64_t total{operands[0]};
  for (std::size_t idx{1}; idx < operators.size(); ++idx) {
    switch (operators[idx]) {
    case '+': {
      total += operands[idx];
      continue;
    }
    case '*': {
      total *= operands[idx];
      continue;
    }
    case '|': {
      total = total * pow10(get_num_digits(operands[idx])) + operands[idx];
      continue;
    }
    default: {
      ASSERT(false, "unreachable", operators[idx]);
      std::unreachable();
    }
    }
  }
  return total;
}

bool
advance_operators(std::vector<char> &operators) {
  for (std::size_t idx{1}; idx < operators.size(); ++idx) {
    switch (operators[idx]) {
    case '+': {
      operators[idx] = '*';
      return true;
    }
    case '*': {
      operators[idx] = '|';
      return true;
    }
    case '|': {
      operators[idx] = '+';
      continue;
    }
    default: {
      ASSERT(false, "unreachable", operators[idx]);
      std::unreachable();
    }
    }
  }
  return false;
}
