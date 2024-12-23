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

  std::uint64_t m_pc{};
  std::vector<std::uint64_t> m_output;

  Computer(std::size_t rega,
           std::size_t regb,
           std::size_t regc,
           std::vector<std::uint64_t> program)
      : m_rega(rega),
        m_regb(regb),
        m_regc(regc),
        m_program(std::move(program)) {}

  [[nodiscard]] std::uint64_t
  combo_op(std::size_t op) const {
    switch (op) {
    case 0:
    case 1:
    case 2:
    case 3: {
      return op;
    }
    case 4: {
      return m_rega;
    }
    case 5: {
      return m_regb;
    }
    case 6: {
      return m_regc;
    }
    default: {
      UNREACHABLE(op);
    }
    }
  }

  void
  adv(std::size_t op) {
    m_rega = m_rega / (1LU << combo_op(op));
    m_pc += 2;
  }

  void
  bxl(std::size_t op) {
    m_regb = m_regb ^ op;
    m_pc += 2;
  }

  void
  bst(std::size_t op) {
    m_regb = combo_op(op) & 0b111;
    m_pc += 2;
  }

  void
  jnz(std::size_t op) {
    m_pc = m_rega == 0 ? m_pc + 2 : op;
  }

  void
  bxc() {
    m_regb = m_regb ^ m_regc;
    m_pc += 2;
  }

  void
  out(std::size_t op) {
    m_output.emplace_back(combo_op(op) & 0b111);
    m_pc += 2;
  }

  void
  bdv(std::size_t op) {
    m_regb = m_rega / (1LU << combo_op(op));
    m_pc += 2;
  }

  void
  cdv(std::size_t op) {
    m_regc = m_rega / (1LU << combo_op(op));
    m_pc += 2;
  }

  std::string
  run() {
    while (m_pc < m_program.size()) {
      std::uint64_t ins = m_program[m_pc];
      std::uint64_t op = m_program[m_pc + 1];
      switch (ins) {
      case 0: {
        adv(op);
        break;
      }
      case 1: {
        bxl(op);
        break;
      }
      case 2: {
        bst(op);
        break;
      }
      case 3: {
        jnz(op);
        break;
      }
      case 4: {
        bxc();
        break;
      }
      case 5: {
        out(op);
        break;
      }
      case 6: {
        bdv(op);
        break;
      }
      case 7: {
        cdv(op);
        break;
      }
      default: {
        UNREACHABLE(ins);
      }
      }
    }

    std::stringstream ss;
    ss << m_output[0];
    for (std::size_t val : m_output | std::views::drop(1)) {
      ss << ',' << val;
    }
    return ss.str();
  }
};

namespace
{
void
tests();
Computer
parse_program(std::ranges::range auto &&lines);
std::string
program_output(std::ranges::range auto &&lines);
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

  fmt::println("{}", program_output(lines));
  return 0;
}

namespace
{
void
tests() {
  using namespace std::literals::string_view_literals;
  {
    auto const lines = std::array{
        "Register A: 729"sv,
        "Register B: 0"sv,
        "Register C: 0"sv,
        ""sv,
        "Program: 0,1,5,4,3,0"sv,
    };
    ASSERT(program_output(lines) == "4,6,3,5,6,3,5,2,1,0");
  }
}

std::string
program_output(std::ranges::range auto &&lines) {
  Computer program = parse_program(lines);
  return program.run();
}

Computer
parse_program(std::ranges::range auto &&lines) {
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
