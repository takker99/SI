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

namespace SI::detail {

/// Concept to check if a type behaves like std::ratio
/// std does not have a ratio concept, so we provide a minimal check
template <typename R>
concept RatioLike = requires {
  { R::num };
  { R::den };
};

/// Concept to check if a type is a unit_t
/// This checks for the presence of required nested types without requiring
/// the type to be complete
template <typename U>
concept UnitLike = requires {
  typename U::ratio;
  typename U::internal_type;
  typename U::exponent;
  typename U::symbol;
  { U::symbol::value } -> std::convertible_to<char>;
};

} // namespace SI::detail
