#include "fmt/core.h"
#include "libassert/assert.hpp"
#include <cstdint>
#include <fstream>
#include <map>
#include <ranges>
#include <string>
#include <string_view>
#include <utility>

#include "utility.hpp"

static constexpr auto UNINIT = std::numeric_limits<unsigned>::max();

static void
tests();
static std::uint64_t
sum_of_score_of_trailheads(std::ranges::range auto &&lines);
static std::vector<unsigned>
generate_full_disk(std::string_view disk_map);
static void
defragment_disk(std::vector<unsigned> &full_disk);
static std::uint64_t
compute_disk_checksum(std::vector<unsigned> const &full_disk);

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

  fmt::println("{}", sum_of_score_of_trailheads(lines));
  return 0;
}

void
tests() {
  using namespace std::literals::string_view_literals;
  {
    auto const lines = std::array{
        "0123"sv,
        "1234"sv,
        "8765"sv,
        "9876"sv,
    };
    ASSERT(sum_of_score_of_trailheads(lines) == 1);
  }
  {
    auto const lines = std::array{
        "...0..."sv,
        "...1..."sv,
        "...2..."sv,
        "6543456"sv,
        "7.....7"sv,
        "8.....8"sv,
        "9.....9"sv,
    };
    ASSERT(sum_of_score_of_trailheads(lines) == 2);
  }
  {
    auto const lines = std::array{
        "..90..9"sv,
        "...1.98"sv,
        "...2..7"sv,
        "6543456"sv,
        "765.987"sv,
        "876...."sv,
        "987...."sv,
    };
    ASSERT(sum_of_score_of_trailheads(lines) == 4);
  }
  {
    auto const lines = std::array{
        "10..9.."sv,
        "2...8.."sv,
        "3...7.."sv,
        "4567654"sv,
        "...8..3"sv,
        "...9..2"sv,
        ".....01"sv,
    };
    ASSERT(sum_of_score_of_trailheads(lines) == 3);
  }
  {
    auto const lines = std::array{
        "89010123"sv,
        "78121874"sv,
        "87430965"sv,
        "96549874"sv,
        "45678903"sv,
        "32019012"sv,
        "01329801"sv,
        "10456732"sv,
    };
    ASSERT(sum_of_score_of_trailheads(lines) == 36);
  }
}

std::uint64_t
sum_of_score_of_trailheads(std::ranges::range auto &&lines) {
  return 0;
}

std::vector<unsigned>
generate_full_disk(std::string_view disk_map) {
  std::vector<unsigned> full_disk(disk_map.size() * 9, UNINIT);
  auto disk_it{full_disk.begin()};
  unsigned file_id{};
  for (std::size_t idx = 0; idx != disk_map.size(); ++idx) {
    unsigned block_size = char_to_int(disk_map[idx]);
    if (idx % 2 == 0) {
      std::fill_n(disk_it, block_size, file_id);
      ++file_id;
    } else {
      std::fill_n(disk_it, block_size, UNINIT);
    }
    std::advance(disk_it, block_size);
  }
  return full_disk;
}

void
defragment_disk(std::vector<unsigned> &full_disk) {
  std::size_t left{0};
  std::size_t right{full_disk.size() - 1};

  while (left < right) {
    // advance left until we find an empty space
    while (left < right && full_disk[left] != UNINIT) {
      ++left;
    }
    // advance right until we find a non-empty space
    while (left < right && full_disk[right] == UNINIT) {
      --right;
    }
    std::swap(full_disk[left], full_disk[right]);
    ++left;
    --right;
  }
}

std::uint64_t
compute_disk_checksum(std::vector<unsigned> const &full_disk) {
  std::uint64_t checksum{};
  for (std::size_t idx{0}; idx < full_disk.size(); ++idx) {
    if (full_disk[idx] == UNINIT) {
      break;
    }
    checksum += idx * full_disk[idx];
  }
  return checksum;
}
