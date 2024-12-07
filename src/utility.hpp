#ifndef UTILITY_HPP
#define UTILITY_HPP

#include <cstdint>
#include <string_view>
#include <vector>

bool
is_digit(char ch);

uint8_t
char_to_int(char ch);

std::size_t
str_to_int(std::string_view sv);

std::vector<std::string_view>
split(std::string_view sv, std::string_view delim = " ");

std::uint8_t
get_num_digits(std::uint64_t num);

std::uint64_t
pow10(std::uint8_t exp);

#endif // UTILITY_HPP
