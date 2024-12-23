#include "utility.hpp"
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

struct Computer
{
  std::uint64_t m_rega{};
  std::uint64_t m_regb{};
  std::uint64_t m_regc{};
  std::vector<std::uint64_t> m_program;

  Computer(std::size_t rega,
           std::size_t regb,
           std::size_t regc,
           std::vector<std::uint64_t> program)
      : m_rega(rega),
        m_regb(regb),
        m_regc(regc),
        m_program(std::move(program)) {}

  std::uint64_t
  min_rega() {
    // this was produces by unrolling the assembly of the program, so it doesn't
    // work for all programs
    // Program: 2,4,1,5,7,5,1,6,4,3,5,5,0,3,3,0
    //
    // bst 4  // regb = rega & 0b111
    // bxl 5  // regb = regb ^ 0b101
    // cdv 5  // regc = rega >> regb
    // bxl 6  // regb = regb ^ 0b110
    // bxc 3  // regb = regb ^ regc
    // out 5  // out regb & 0b111
    // adv 3  // rega = rega / (1 << 3)
    // jnz 0
    //
    // bxl 5  // regb = (rega & 0b111) ^ 0b101
    // cdv 5  // regc = rega >> regb
    // bxl 6  // regb = regb ^ 0b110
    // bxc 3  // regb = regb ^ regc
    // out 5  // out regb & 0b111
    // adv 3  // rega = rega / (1 << 3)
    // jnz 0
    //
    // cdv 5  // regc = rega >> ((rega & 0b111) ^ 0b101)
    // bxl 6  // regb = ((rega & 0b111) ^ 0b101) ^ 0b110
    // bxc 3  // regb = regb ^ regc
    // out 5  // out regb & 0b111
    // adv 3  // rega = rega / (1 << 3)
    // jnz 0
    //
    // bxl 6  // regb = ((rega & 0b111) ^ 0b101) ^ 0b110
    // bxc 3  // regb = regb ^ (rega >> ((rega & 0b111) ^ 0b101))
    // out 5  // out regb & 0b111
    // adv 3  // rega = rega / (1 << 3)
    // jnz 0
    //
    // bxc 3  // regb = (((rega & 0b111) ^ 0b101) ^ 0b110) ^ (rega >> ((rega & 0b111) ^ 0b101))
    // out 5  // out regb & 0b111
    // adv 3  // rega = rega / (1 << 3)
    // jnz 0
    //
    // out 5  // out ((((rega & 0b111) ^ 0b101) ^ 0b110) ^ (rega >> ((rega & 0b111) ^ 0b101))) & 0b111
    // adv 3  // rega = rega / (1 << 3)
    // jnz 0
    //
    // out 5  // out ((((rega & 0b111) ^ 0b101) ^ 0b110) ^ (rega >> ((rega & 0b111) ^ 0b101))) & 0b111
    // adv 3  // rega = rega >> 3
    // jnz 0

    std::vector<std::uint64_t> values;
    values.emplace_back(m_program.back());
    for (std::uint64_t idx = 0; idx < m_program.size(); ++idx) {
      std::vector<std::uint64_t> next_values;
      for (auto value : values) {
        for (std::uint64_t i = 0b000; i <= 0b111; ++i) {
          std::uint64_t val = (value << 3UL) + i;
          auto out = ((((val & 0b111) ^ 0b101) ^ 0b110)
                      ^ (val >> ((val & 0b111) ^ 0b101)))
                     & 0b111;
          if (out == m_program[m_program.size() - idx - 1]) {
            next_values.emplace_back(val);
          }
        }
      }
      values = std::move(next_values);
    }
    return values[0];
  }
};

namespace
{
Computer
parse_computer(std::ranges::range auto &&lines);
std::uint64_t
min_rega(std::ranges::range auto &&lines);
} // namespace

int
main(int argc, char const *const *argv) {
  // tests();

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

  fmt::println("{}", min_rega(lines));
  return 0;
}

namespace
{
std::uint64_t
min_rega(std::ranges::range auto &&lines) {
  Computer computer = parse_computer(lines);
  return computer.min_rega();
}

Computer
parse_computer(std::ranges::range auto &&lines) {
  auto rega = scn::scan<std::uint64_t>(lines[0], "Register A: {}")->value();
  auto regb = scn::scan<std::uint64_t>(lines[1], "Register B: {}")->value();
  auto regc = scn::scan<std::uint64_t>(lines[2], "Register C: {}")->value();
  auto program =
      split(scn::scan<std::string>(lines[4], "Program: {}")->value(), ",")
      | std::views::transform([](std::string_view sv) {
          return scn::scan<std::uint64_t>(sv, "{}")->value();
        })
      | std::ranges::to<std::vector<std::uint64_t>>();

  return {rega, regb, regc, program};
}
} // namespace
