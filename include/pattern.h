// MIT License
//
// Copyright (c) 2023 xxkat
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#pragma once

#include <algorithm>
#include <array>
#include <cstdint>
#include <ranges>
#include <span>
#include <type_traits>
#include <utility>

namespace pattern {

constexpr char kWildcardChar{'.'};

template <size_t N>
struct PatternString {
  constexpr PatternString(const char (&str)[N]) : m_storage{std::to_array(str)} {}

  constexpr size_t size() const {
    return m_storage.size();
  }

  constexpr const char *begin() const {
    return m_storage.begin();
  }

  constexpr const char *end() const {
    return m_storage.end();
  }

  constexpr char operator[](size_t index) const {
    return m_storage[index];
  }

  std::array<char, N> m_storage{};
};

struct PatternByte {
  constexpr bool operator==(uint8_t byte) const {
    return m_value == byte || m_wildcard;
  }

  uint8_t m_value{};
  bool m_wildcard{};
};

template <size_t N>
using PatternArray = std::array<PatternByte, N>;

template <typename T>
concept IsPattern = std::is_same_v<std::ranges::range_value_t<T>, PatternByte>;

constexpr uint8_t char_to_hex(char ch) {
  if (ch >= '0' && ch <= '9') {
    return static_cast<uint8_t>(ch - '0');
  } else if (ch >= 'a' && ch <= 'f') {
    return static_cast<uint8_t>(ch - 'a') + 10;
  } else if (ch >= 'A' && ch <= 'F') {
    return static_cast<uint8_t>(ch - 'A') + 10;
  } else {
    std::unreachable();
  }
}

constexpr bool is_hex(char ch) {
  return (ch >= '0' && ch <= '9') || (ch >= 'a' && ch <= 'f') || (ch >= 'A' && ch <= 'F');
}

constexpr uint8_t to_byte(char x, char y) {
  return (char_to_hex(x) << 4) | char_to_hex(y);
}

template <PatternString Str>
constexpr auto count_bytes{[]() {
  size_t count{0};
  for (size_t i{0}; i < Str.size(); ++i) {
    if (Str[i] == kWildcardChar) {
      ++count;
    } else if (is_hex(Str[i]) && is_hex(Str[i + 1])) {
      ++count, ++i;
    }
  }
  return count;
}()};

template <PatternString Str>
constexpr auto build{[]() {
  PatternArray<count_bytes<Str>> result{};
  for (size_t i{0}, j{0}; i < Str.size(); ++i) {
    if (Str[i] == kWildcardChar) {
      result[j++].m_wildcard = true;
    } else if (is_hex(Str[i]) && is_hex(Str[i + 1])) {
      result[j++].m_value = to_byte(Str[i], Str[i + 1]);
      ++i;
    }
  }
  return result;
}()};

template <IsPattern auto Pattern>
uintptr_t find(std::span<const uint8_t> memory) {
  const auto it{std::search(memory.begin(), memory.end(), Pattern.begin(), Pattern.end())};
  return it != memory.end() ? reinterpret_cast<uintptr_t>(&*it) : 0;
}

}  // namespace pattern