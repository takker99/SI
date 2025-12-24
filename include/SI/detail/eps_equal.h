/**
 * This file is part of "SI" version 2.5.4
 * A header only c++ library that provides type safety and user defined literals
 * for handling physical values defined in the International System of
 * Units
 *
 * https://github.com/bernedom/SI
 *
 * SPDX-License-Identifier: MIT
 *
 **/
#pragma once

#include <concepts>
#include <limits>
#include <numeric>

namespace SI::detail {

/// @todo make eps_equal take different types with similar properties
template <std::floating_point T>
constexpr bool eps_equals(const T &lhs, const T &rhs) {

  return (lhs - rhs) < std::numeric_limits<T>::epsilon() &&
         (lhs - rhs) > -std::numeric_limits<T>::epsilon();
  // return std::abs(lhs - rhs) < std::numeric_limits<T>::epsilon();
}

} // namespace SI::detail