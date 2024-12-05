#include <fmt/core.h>
#include <fstream>
#include <regex>
#include <string>

#include <libassert/assert.hpp>

std::uint64_t
collect_all_muls(std::string correpted) {
  std::uint64_t total{};
  std::regex mul_regex(R"(mul\((\d+),(\d+)\))");
  for (std::smatch sm; std::regex_search(correpted, sm, mul_regex);) {
    auto factor1 = sm[1].str();
    auto factor2 = sm[2].str();
    std::uint64_t factor1_num{};
    std::uint64_t factor2_num{};
    std::from_chars(factor1.c_str(),
                    factor1.c_str() + factor1.size(),
                    factor1_num);
    std::from_chars(factor2.c_str(),
                    factor2.c_str() + factor2.size(),
                    factor2_num);
    total += factor1_num * factor2_num;
    correpted = sm.suffix();
  }
  return total;
}

void
tests() {
  ASSERT(
      collect_all_muls(
          "xmul(2,4)%&mul[3,7]!@^do_not_mul(5,5)+mul(32,64]then(mul(11,8)mul(8,5))")
      == 161);
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

  std::uint64_t total{};
  for (std::string line; std::getline(infile, line);) {
    total += collect_all_muls(line);
  }
  fmt::println("{}", total);
  return 0;
}
