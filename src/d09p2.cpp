#include "fmt/core.h"
#include "libassert/assert.hpp"
#include <cstdint>
#include <fstream>
#include <ranges>
#include <string>
#include <string_view>
#include <utility>

#include "utility.hpp"

static constexpr auto UNINIT = std::numeric_limits<unsigned>::max();

struct File
{
  bool is_file;
  std::uint64_t id;
  std::uint64_t size;
};

static void
tests();
static std::uint64_t
compute_filesystem_checksum(std::ranges::range auto &&lines);
static std::vector<File>
generate_full_disk(std::string_view disk_map);
static void
defragment_disk(std::vector<File> &full_disk);
void
merge_spaces(std::vector<File> &full_disk);
static std::uint64_t
compute_disk_checksum(std::vector<File> const &full_disk);

void
dump_full_disk(std::vector<File> const &full_disk) {
  for (File const &f : full_disk) {
    ASSERT(f.size < 10);
    for (std::uint64_t idx = 0; idx < f.size; ++idx) {
      if (f.id == UNINIT) {
        fmt::print(".");
      } else {
        fmt::print("{}", f.id);
      }
    }
  }
  fmt::println("");
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

  fmt::println("{}", compute_filesystem_checksum(lines));
  return 0;
}

void
tests() {
  using namespace std::literals::string_view_literals;
  auto lines = std::vector{{"2333133121414131402"sv}};
  ASSERT(compute_filesystem_checksum(lines) == 2858);
}

std::uint64_t
compute_filesystem_checksum(std::ranges::range auto &&lines) {
  auto const &disk_map = lines[0];
  auto full_disk = generate_full_disk(disk_map);
  defragment_disk(full_disk);
  return compute_disk_checksum(full_disk);
}

std::vector<File>
generate_full_disk(std::string_view disk_map) {
  std::vector<File> full_disk;
  std::uint64_t file_id{};
  for (std::size_t idx = 0; idx != disk_map.size(); ++idx) {
    std::uint64_t block_size = char_to_int(disk_map[idx]);
    if (idx % 2 == 0) {
      if (block_size > 0) {
        full_disk.emplace_back(true, file_id, block_size);
      }
      ++file_id;
    } else {
      if (block_size > 0) {
        full_disk.emplace_back(false, UNINIT, block_size);
      }
    }
  }
  return full_disk;
}

void
defragment_disk(std::vector<File> &full_disk) {
  std::size_t num_blocks{full_disk.size()};
  for (std::size_t block_idx{num_blocks - 1}; block_idx != 0; --block_idx) {
    File &file_block{full_disk[block_idx]};
    if (!file_block.is_file) {
      continue;
    }

    for (std::size_t find_idx{0}; find_idx < block_idx; ++find_idx) {
      File &space_block{full_disk[find_idx]};
      if (space_block.is_file) {
        continue;
      }
      if (space_block.size < file_block.size) {
        continue;
      }

      std::swap(space_block, file_block);
      std::uint64_t remaining_space = file_block.size - space_block.size;
      file_block.size = space_block.size;
      if (remaining_space != 0) {
        auto next_it{std::next(full_disk.begin(), find_idx + 1)};
        full_disk.insert(next_it, File{false, UNINIT, remaining_space});
        ++block_idx;
      }
      merge_spaces(full_disk);
      break;
    }
  }
}

void
merge_spaces(std::vector<File> &full_disk) {
  for (std::size_t idx{}; idx < full_disk.size(); ++idx) {
    if (full_disk[idx].is_file) {
      continue;
    }
    for (std::size_t sub_idx{idx + 1}; sub_idx < full_disk.size(); ++sub_idx) {
      if (full_disk[sub_idx].is_file) {
        break;
      }
      full_disk[idx].size += full_disk[sub_idx].size;
      full_disk.erase(std::next(full_disk.begin(), sub_idx));
      --sub_idx;
    }
  }
}

std::uint64_t
compute_disk_checksum(std::vector<File> const &full_disk) {
  std::uint64_t checksum{};
  std::size_t disk_idx{};
  for (File const &f : full_disk) {
    for (std::size_t idx{0}; idx < f.size; ++idx) {
      if (f.is_file) {
        checksum += disk_idx * f.id;
      }
      ++disk_idx;
    }
  }
  return checksum;
}
