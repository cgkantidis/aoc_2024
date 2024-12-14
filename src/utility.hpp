#ifndef UTILITY_HPP
#define UTILITY_HPP

#include <charconv>
#include <cstdint>
#include <string_view>
#include <utility> // unreachable
#include <vector>

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

  std::unreachable();
  return 0;
}

std::vector<std::string_view>
split(std::string_view sv, std::string_view delim = " ");

std::uint8_t
get_num_digits(std::uint64_t num);

std::uint64_t
pow10(std::uint8_t exp);

#endif // UTILITY_HPP
