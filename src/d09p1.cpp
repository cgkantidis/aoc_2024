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
compute_filesystem_checksum(std::ranges::range auto &&lines);
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

  fmt::println("{}", compute_filesystem_checksum(lines));
  return 0;
}

void
tests() {
  using namespace std::literals::string_view_literals;
  auto lines = std::vector{{"2333133121414131402"sv}};
  ASSERT(compute_filesystem_checksum(lines) == 1928);
}

std::uint64_t
compute_filesystem_checksum(std::ranges::range auto &&lines) {
  auto const &disk_map = lines[0];
  auto full_disk = generate_full_disk(disk_map);
  defragment_disk(full_disk);
  return compute_disk_checksum(full_disk);
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
