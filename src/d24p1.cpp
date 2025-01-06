#include "BS_thread_pool.hpp"
#include "utility.hpp" // str_to_int
#include <array> // std::array
#include <bitset>
#include <cstdint> // std::uint64_t
#include <fmt/core.h> // fmt::print
#include <fmt/ranges.h> // fmt::print
#include <fstream> // std::ifstream
#include <libassert/assert.hpp> // ASSERT
#include <ranges> // std::views::enumerate
#include <string> // std::string
#include <unordered_map> // std::unordered_map
#include <unordered_set> // std::unordered_set
#include <vector> // std::vector

enum class Gate
{
  AND,
  OR,
  XOR
};

using ulong = std::uint64_t;
using known_signals_t = std::unordered_map<std::string, bool>;
using unknown_signals_t =
    std::unordered_map<std::string, std::tuple<Gate, std::string, std::string>>;
using namespace std::literals::string_view_literals;

namespace
{
void
tests();
ulong
get_circuit_output(std::ranges::range auto &&lines);
std::pair<known_signals_t, unknown_signals_t>
parse_signals(std::ranges::range auto &&lines);
Gate
str_to_gate(std::string_view sv);
void
propagate_known_signals(known_signals_t &known_signals,
                        unknown_signals_t &unknown_signals);
ulong
convert_known_signals(known_signals_t const &known_signals);
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

  fmt::println("{}", get_circuit_output(lines));
  return 0;
}

namespace
{
void
tests() {
  {
    auto const lines = std::array{
        "x00: 1"sv,
        "x01: 1"sv,
        "x02: 1"sv,
        "y00: 0"sv,
        "y01: 1"sv,
        "y02: 0"sv,
        ""sv,
        "x00 AND y00 -> z00"sv,
        "x01 XOR y01 -> z01"sv,
        "x02 OR y02 -> z02"sv,
    };
    ASSERT(get_circuit_output(lines) == 4);
  }
  {
    auto const lines = std::array{
        "x00: 1"sv,
        "x01: 0"sv,
        "x02: 1"sv,
        "x03: 1"sv,
        "x04: 0"sv,
        "y00: 1"sv,
        "y01: 1"sv,
        "y02: 1"sv,
        "y03: 1"sv,
        "y04: 1"sv,
        ""sv,
        "ntg XOR fgs -> mjb"sv,
        "y02 OR x01 -> tnw"sv,
        "kwq OR kpj -> z05"sv,
        "x00 OR x03 -> fst"sv,
        "tgd XOR rvg -> z01"sv,
        "vdt OR tnw -> bfw"sv,
        "bfw AND frj -> z10"sv,
        "ffh OR nrd -> bqk"sv,
        "y00 AND y03 -> djm"sv,
        "y03 OR y00 -> psh"sv,
        "bqk OR frj -> z08"sv,
        "tnw OR fst -> frj"sv,
        "gnj AND tgd -> z11"sv,
        "bfw XOR mjb -> z00"sv,
        "x03 OR x00 -> vdt"sv,
        "gnj AND wpb -> z02"sv,
        "x04 AND y00 -> kjc"sv,
        "djm OR pbm -> qhw"sv,
        "nrd AND vdt -> hwm"sv,
        "kjc AND fst -> rvg"sv,
        "y04 OR y02 -> fgs"sv,
        "y01 AND x02 -> pbm"sv,
        "ntg OR kjc -> kwq"sv,
        "psh XOR fgs -> tgd"sv,
        "qhw XOR tgd -> z09"sv,
        "pbm OR djm -> kpj"sv,
        "x03 XOR y03 -> ffh"sv,
        "x00 XOR y04 -> ntg"sv,
        "bfw OR bqk -> z06"sv,
        "nrd XOR fgs -> wpb"sv,
        "frj XOR qhw -> z04"sv,
        "bqk OR frj -> z07"sv,
        "y03 OR x01 -> nrd"sv,
        "hwm AND bqk -> z03"sv,
        "tgd XOR rvg -> z12"sv,
        "tnw OR pbm -> gnj"sv,
    };
    ASSERT(get_circuit_output(lines) == 2024);
  }
}

ulong
get_circuit_output(std::ranges::range auto &&lines) {
  auto [known_signals, unknown_signals] = parse_signals(lines);
  propagate_known_signals(known_signals, unknown_signals);
  return convert_known_signals(known_signals);
}

std::pair<known_signals_t, unknown_signals_t>
parse_signals(std::ranges::range auto &&lines) {
  known_signals_t known_signals;
  unknown_signals_t unknown_signals;

  bool found_empty_line{};
  for (auto const &line : lines) {
    if (line.empty()) {
      found_empty_line = true;
      continue;
    }
    if (!found_empty_line) {
      auto tokens = split(line, ": ");
      known_signals[std::string(tokens[0])] = tokens[1] == "1"sv;
      continue;
    }
    auto tokens = split(line);
    unknown_signals[std::string(tokens[4])] = {str_to_gate(tokens[1]),
                                               std::string(tokens[0]),
                                               std::string(tokens[2])};
  }
  return {known_signals, unknown_signals};
}

Gate
str_to_gate(std::string_view sv) {
  if (sv == "AND") {
    return Gate::AND;
  }
  if (sv == "OR") {
    return Gate::OR;
  }
  if (sv == "XOR") {
    return Gate::XOR;
  }
  UNREACHABLE(sv);
}

void
propagate_known_signals(known_signals_t &known_signals,
                        unknown_signals_t &unknown_signals) {
  while (!unknown_signals.empty()) {
    std::vector<std::string> to_remove;
    for (auto const &[signal, tuple] : unknown_signals) {
      auto const &[gate, in1, in2] = tuple;
      auto find_in1 = known_signals.find(in1);
      if (find_in1 == known_signals.end()) {
        continue;
      }
      auto find_in2 = known_signals.find(in2);
      if (find_in2 == known_signals.end()) {
        continue;
      }

      switch (gate) {
      case Gate::AND: {
        known_signals[signal] = find_in1->second && find_in2->second;
        break;
      }
      case Gate::OR: {
        known_signals[signal] = find_in1->second || find_in2->second;
        break;
      }
      case Gate::XOR: {
        known_signals[signal] = find_in1->second ^ find_in2->second;
        break;
      }
      }

      to_remove.emplace_back(signal);
    }

    for (auto const &rem : to_remove) {
      unknown_signals.erase(rem);
    }
  }
}

ulong
convert_known_signals(known_signals_t const &known_signals) {
  std::bitset<64> output;
  for (auto const &[signal, value] : known_signals) {
    if (signal[0] != 'z') {
      continue;
    }
    output[str_to_int<ulong>(signal.substr(1))] = value;
  }
  return output.to_ulong();
}

} // namespace
