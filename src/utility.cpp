#include <charconv>
#include <utility> // unreachable

#include "utility.hpp"

std::size_t
str_to_int(std::string_view sv) {
  std::size_t result{};
  auto [ptr, ec] = std::from_chars(sv.data(), sv.data() + sv.size(), result);

  if (ec == std::errc()) {
    return result;
  }

  std::unreachable();
  return 0;
}

std::vector<std::string_view>
split(std::string_view sv, std::string_view delim) {
  std::vector<std::string_view> tokens;
  for (std::size_t right = sv.find(delim); right != std::string_view::npos;
       right = sv.find(delim)) {
    if (right == 0) {
      sv = sv.substr(right + delim.size());
      continue;
    }
    tokens.emplace_back(sv.substr(0, right));
    sv = sv.substr(right + delim.size());
  }
  if (!sv.empty()) {
    tokens.emplace_back(sv);
  }
  return tokens;
}
