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

} // namespace SI::detail
