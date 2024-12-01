#ifndef UTILITY_HPP
#define UTILITY_HPP

#include <string_view>
#include <vector>

std::size_t
str_to_int(std::string_view sv);

std::vector<std::string_view>
split(std::string_view sv, std::string_view delim = " ");

#endif // UTILITY_HPP
