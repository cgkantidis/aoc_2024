#ifndef UTILITY_HPP
#define UTILITY_HPP

#include <charconv> // std::from_chars
#include <cstdint> // std::uint64_t
#include <libassert/assert.hpp> // UNREACHABLE
#include <string_view> // std::string_view
#include <vector> // std::vector

bool
is_digit(char ch);

uint8_t
char_to_int(char ch);

template <typename T>
T
str_to_int(std::string_view sv) {
  T result{};
  auto [ptr, ec] = std::from_chars(sv.data(), sv.data() + sv.size(), result);

  if (ec == std::errc()) {
    return result;
  }

  UNREACHABLE();
}

std::vector<std::string_view>
split(std::string_view sv, std::string_view delim = " ");

std::uint8_t
get_num_digits(std::uint64_t num);

std::uint64_t
pow10(std::uint8_t exp);

struct Location
{
  std::size_t row;
  std::size_t col;
};

/// we need the operator==() to resolve hash collisions
bool
operator==(Location const &lhs, Location const &rhs) {
  return lhs.row == rhs.row && lhs.col == rhs.col;
}

/// we can use the hash_combine variadic-template function, to combine multiple
/// hashes into a single one
template <typename T, typename... Rest>
constexpr void
hash_combine(std::size_t &seed, T const &val, Rest const &...rest) {
  constexpr size_t hash_mask{0x9e3779b9};
  constexpr size_t lsh{6};
  constexpr size_t rsh{2};
  seed ^= std::hash<T>{}(val) + hash_mask + (seed << lsh) + (seed >> rsh);
  (hash_combine(seed, rest), ...);
}

/// custom specialization of std::hash injected in namespace std
template <>
struct std::hash<Location>
{
  std::size_t
  operator()(Location const &loc) const noexcept {
    std::size_t h1 = std::hash<std::size_t>{}(loc.row);
    std::size_t h2 = std::hash<std::size_t>{}(loc.col);

    std::size_t ret_val = 0;
    hash_combine(ret_val, h1, h2);
    return ret_val;
  }
};

#endif // UTILITY_HPP
