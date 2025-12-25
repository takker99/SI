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

#ifdef SI_DISABLE_IMPLICIT_RATIO_CONVERSION
#define SI_ENABLE_IMPLICIT_RATIO_CONVERSION false
#else
#define SI_ENABLE_IMPLICIT_RATIO_CONVERSION true
#endif

#include "concepts.h"
#include "detail.h"
#include "eps_equal.h"
#include "unit_cast.h"

#include <concepts>
#include <ratio>
#include <type_traits>

/// Namespace containing all SI units
namespace SI::detail {

template <UnitLike _unit_lhs, UnitLike _unit_rhs>
  requires std::convertible_to<typename _unit_lhs::internal_type, typename _unit_rhs::internal_type> &&
           (_unit_lhs::symbol::value == _unit_rhs::symbol::value)
struct unit_with_common_ratio;

/// @todo add in-place unit_cast for move operators
/// @todo add logarithmic units (decibel)
/// @todo add constants for speed of light, planck constant, elementary charge,
/// boltzmann constant, avogadro constant and  luminous efficacy of
/// monochromatic radiation of frequency

/**
 * @brief base template class for holding values of type _type to be multiplied
 *with a ratio _ratio
 *
 * @tparam _symbol dimension symbol of the unit (see
 * https://en.wikipedia.org/wiki/SI_base_unit)
 * @tparam _ratio Ratio in relation to the base unit of a SI unit
 * @tparam _type internal representation of the value
 * @tparam _exponent the exponent to the unit (i.e. length ==  m^1, area == m^2,
 *volume = m^3)
 **/
template <char _symbol, typename _exponent, typename _type,
          typename _ratio = std::ratio<1>>
  requires std::is_arithmetic_v<_type> && RatioLike<_exponent> && RatioLike<_ratio>
struct unit_t {
  using ratio = _ratio;
  using internal_type = _type;
  using exponent = _exponent;
  using symbol = std::integral_constant<char, _symbol>;

  /// Construct with value v
  explicit constexpr unit_t(_type v) : value_{v} {}
  constexpr unit_t() = default;
  constexpr unit_t(const unit_t &) = default;
  constexpr unit_t(unit_t &&) = default;

  /// construct from other unit with implicitly convertible type
  template <typename _type_rhs>
    requires std::convertible_to<_type_rhs, _type>
  constexpr unit_t(const unit_t<_symbol, _exponent, _type_rhs, _ratio> &rhs)
      : value_(rhs.value()) {
  }

  ~unit_t() = default;

  template <typename _rhs_type, typename _rhs_ratio>
    requires RatioLike<_rhs_ratio>
  constexpr unit_t(const unit_t<_symbol, _exponent, _rhs_type, _rhs_ratio> &rhs)
      : value_{
            unit_cast<unit_t<_symbol, _exponent, _type, _ratio>>(rhs).value()} {
    static_assert(
        SI_ENABLE_IMPLICIT_RATIO_CONVERSION ||
            std::ratio_equal_v<ratio, _rhs_ratio>,
        "Implicit ratio conversion disabled, convert before assigning");
  }

  template <typename _rhs_ratio>
    requires RatioLike<_rhs_ratio>
  constexpr unit_t(unit_t<_symbol, _exponent, _type, _rhs_ratio> &&rhs)
      : value_{
            std::move(unit_cast<unit_t<_symbol, _exponent, _type, _ratio>>(rhs)
                          .value())} {
    static_assert(
        SI_ENABLE_IMPLICIT_RATIO_CONVERSION ||
            std::ratio_equal_v<ratio, _rhs_ratio>,
        "Implicit ratio conversion disabled, convert before assigning");
  }

  /// returns the stored value as raw type
  constexpr _type value() const { return value_; }

  /// returns a new instance of the unit with a differend value and explicit
  /// specified underlying type
  template <UnitLike _unit_rhs> 
    requires (std::ratio_equal_v<typename _unit_rhs::exponent, _exponent> &&
              _unit_rhs::symbol::value == _symbol)
  constexpr _unit_rhs as() const {
    return unit_cast<_unit_rhs>(*this);
  }

  /// returns a new instance of the unit with a differend value with the same
  /// underlying type
  template <template <typename _type_rhs> typename _unit_rhs>
    requires UnitLike<_unit_rhs<_type>> &&
             (std::ratio_equal_v<typename _unit_rhs<_type>::exponent, _exponent> &&
              _unit_rhs<_type>::symbol::value == _symbol)
  constexpr _unit_rhs<_type> as() const {
    return unit_cast<_unit_rhs<_type>>(*this);
  }

  ///@todo set as friend to the stream-function
  void setValue(_type v) { value_ = v; }

  /// Assignment for same ratio
  constexpr unit_t &operator=(const unit_t &rhs) = default;

  /// Move assignment for same ratio
  constexpr unit_t &operator=(unit_t &&rhs) = default;

  /// Assignment of same unit but different ratio
  template <typename _rhs_ratio>
    requires (!std::ratio_equal_v<_rhs_ratio, _ratio> && RatioLike<_rhs_ratio>)
  constexpr unit_t &
  operator=(const unit_t<_symbol, _exponent, _type, _rhs_ratio> &rhs) {

    static_assert(
        SI_ENABLE_IMPLICIT_RATIO_CONVERSION ||
            std::ratio_equal_v<ratio, _rhs_ratio>,
        "Implicit ratio conversion disabled, convert before assigning");

    *this = unit_cast<unit_t<_symbol, _exponent, _type, _ratio>>(rhs);
    return *this;
  }

  /// Move assignment of same unit but different ratio
  template <typename _rhs_ratio>
    requires (!std::ratio_equal_v<_rhs_ratio, _ratio> && RatioLike<_rhs_ratio>)
  constexpr unit_t &
  operator=(unit_t<_symbol, _exponent, _type, _rhs_ratio> &&rhs) {

    static_assert(
        SI_ENABLE_IMPLICIT_RATIO_CONVERSION ||
            std::ratio_equal_v<ratio, _rhs_ratio>,
        "Implicit ratio conversion disabled, convert before assigning");

    *this =
        std::move(unit_cast<unit_t<_symbol, _exponent, _type, _ratio>>(rhs));
    return *this;
  }

  /// Comparison operator takes considers different ratios, i.e. 1000
  /// micro == 1 milli
  template <typename _rhs_type, typename _rhs_ratio>
    requires RatioLike<_rhs_ratio> && (std::is_integral_v<_type> || std::is_floating_point_v<_type>)
  constexpr bool operator==(
      const unit_t<_symbol, _exponent, _rhs_type, _rhs_ratio> &rhs) const {

    static_assert(
        SI_ENABLE_IMPLICIT_RATIO_CONVERSION ||
            std::ratio_equal_v<ratio, _rhs_ratio>,
        "Implicit ratio conversion disabled, convert before comparing");

    using gcd_unit = typename unit_with_common_ratio<
        typename std::remove_reference<decltype(rhs)>::type,
        typename std::remove_reference<decltype(*this)>::type>::type;

    if constexpr (std::is_integral_v<_type>) {

      return unit_cast<gcd_unit>(rhs).value() ==
             unit_cast<gcd_unit>(*this).value();
    } else {
      return detail::eps_equals(unit_cast<gcd_unit>(rhs).value(),
                                unit_cast<gcd_unit>(*this).value());
    }
  }

  /// compares two values, considers different ratios.
  template <typename _rhs_type, typename _rhs_ratio>
    requires RatioLike<_rhs_ratio>
  constexpr bool operator!=(
      const unit_t<_symbol, _exponent, _rhs_type, _rhs_ratio> &rhs) const {
    return !(*this == rhs);
  }

  template <typename _rhs_type, typename _rhs_ratio>
    requires RatioLike<_rhs_ratio>
  constexpr bool operator<(
      const unit_t<_symbol, _exponent, _rhs_type, _rhs_ratio> &rhs) const {
    static_assert(
        SI_ENABLE_IMPLICIT_RATIO_CONVERSION ||
            std::ratio_equal_v<ratio, _rhs_ratio>,
        "Implicit ratio conversion disabled, convert before comparing");

    using gcd_unit = typename unit_with_common_ratio<
        typename std::remove_reference<decltype(rhs)>::type,
        typename std::remove_reference<decltype(*this)>::type>::type;
    return unit_cast<gcd_unit>(*this).value() <
           unit_cast<gcd_unit>(rhs).value();
  }

  template <typename _rhs_type, typename _rhs_ratio>
  constexpr bool operator<=(
      const unit_t<_symbol, _exponent, _rhs_type, _rhs_ratio> &rhs) const {
    return !(*this > rhs);
  }

  template <typename _rhs_type, typename _rhs_ratio>
    requires RatioLike<_rhs_ratio>
  constexpr bool operator>(
      const unit_t<_symbol, _exponent, _rhs_type, _rhs_ratio> &rhs) const {
    static_assert(
        SI_ENABLE_IMPLICIT_RATIO_CONVERSION ||
            std::ratio_equal_v<ratio, _rhs_ratio>,
        "Implicit ratio conversion disabled, convert before comparing");

    using gcd_unit = typename unit_with_common_ratio<
        typename std::remove_reference<decltype(rhs)>::type,
        typename std::remove_reference<decltype(*this)>::type>::type;

    return unit_cast<gcd_unit>(*this).value() >
           unit_cast<gcd_unit>(rhs).value();
  }

  template <typename _rhs_type, typename _rhs_ratio>
  constexpr bool operator>=(
      const unit_t<_symbol, _exponent, _rhs_type, _rhs_ratio> &rhs) const {
    return !(*this < rhs);
  }

  /// multiply with a non-unit scalar
  constexpr unit_t operator*(const _type f) const { return unit_t{value_ * f}; }

  /// multiply with an unit of the same ratio
  template <typename _rhs_exponent, typename _rhs_type>
    requires RatioLike<_rhs_exponent>
  constexpr auto operator*(
      const unit_t<_symbol, _rhs_exponent, _rhs_type, _ratio> &rhs) const {

    return unit_t<_symbol, std::ratio_add<_rhs_exponent, _exponent>, _type,
                  std::ratio_multiply<ratio, _ratio>>{value() * rhs.value()};
  }

  /// multiplication multiply with a same unit, with different exponent
  /// and different ratio
  /// the exponents this and rhs are added, the resulting ratio the ratio
  /// multiplied.
  template <typename _rhs_exponent, typename _rhs_ratio, typename _rhs_type>
    requires RatioLike<_rhs_exponent> && RatioLike<_rhs_ratio>
  constexpr auto operator*(
      const unit_t<_symbol, _rhs_exponent, _rhs_type, _rhs_ratio> &rhs) const {

    static_assert(
        SI_ENABLE_IMPLICIT_RATIO_CONVERSION ||
            std::ratio_equal_v<ratio, _rhs_ratio>,
        "Implicit ratio conversion disabled, convert before comparing");

    return unit_t<_symbol, std::ratio_add<_exponent, _rhs_exponent>, _type,
                  std::ratio_multiply<ratio, _rhs_ratio>>{value_ * rhs.value()};
  }

  /// multiply with a non-unit scalar
  constexpr unit_t &operator*=(const _type scalar) {
    value_ *= scalar;
    return *this;
  }

  /// divide by a non-unit scalar
  constexpr unit_t operator/(const _type f) const { return unit_t{value_ / f}; }

  /// divide with same unit with same ratio but not the same exponent
  /// @returns unit with exponents subtracted from each others
  template <typename _rhs_exponent, typename _rhs_type>
    requires (std::ratio_not_equal_v<_rhs_exponent, _exponent> && RatioLike<_rhs_exponent>)
  constexpr auto operator/(
      const unit_t<_symbol, _rhs_exponent, _rhs_type, _ratio> &rhs) const {
    using rhs_t = typename std::remove_reference<decltype(rhs)>::type;

    return unit_t<_symbol,
                  std::ratio_subtract<_exponent, typename rhs_t::exponent>,
                  _type, std::ratio_divide<ratio, _ratio>>{value_ /
                                                           rhs.value()};
  }

  /// divide with a same unit but different ratios
  /// the ratio of the result is the gcd of the two ratios and the exponents are
  /// subtracted
  template <typename _rhs_exponent, typename _rhs_type, typename _rhs_ratio>
    requires (std::ratio_not_equal_v<_rhs_exponent, _exponent> && RatioLike<_rhs_exponent> && RatioLike<_rhs_ratio>)
  constexpr auto operator/(
      const unit_t<_symbol, _rhs_exponent, _rhs_type, _rhs_ratio> &rhs) const {
    static_assert(
        SI_ENABLE_IMPLICIT_RATIO_CONVERSION ||
            std::ratio_equal_v<ratio, _rhs_ratio>,
        "Implicit ratio conversion disabled, convert before dividing");

    return unit_t<_symbol, std::ratio_subtract<exponent, _rhs_exponent>, _type,
                  std::ratio_divide<ratio, _rhs_ratio>>{value_ / rhs.value()};
  }

  /// divide whit same unit result is a scalar
  template <typename _rhs_type>
  constexpr _type
  operator/(const unit_t<_symbol, _exponent, _rhs_type, _ratio> &rhs) {
    return value() / rhs.value();
  }

  /// if the same units of the same exponent but different ratio are divided
  /// then the result is a scalar
  template <typename _rhs_exponent, typename _rhs_type, typename _rhs_ratio>
    requires (std::ratio_equal_v<_rhs_exponent, exponent> && RatioLike<_rhs_exponent> && RatioLike<_rhs_ratio>)
  constexpr _type operator/(
      const unit_t<_symbol, _rhs_exponent, _rhs_type, _rhs_ratio> &rhs) const {
    static_assert(SI_ENABLE_IMPLICIT_RATIO_CONVERSION ||
                      std::ratio_equal_v<_rhs_ratio, _ratio>,
                  "Implicit ratio conversion disabled, convert to same ratio "
                  "before dividing");

    using gcd_unit = typename unit_with_common_ratio<
        typename std::remove_reference<decltype(*this)>::type,
        typename std::remove_reference<decltype(rhs)>::type>::type;

    return unit_cast<gcd_unit>(*this) / unit_cast<gcd_unit>(rhs);
  }

  /// divide with a non-unit scalar
  constexpr unit_t &operator/=(const _type scalar) {
    value_ /= scalar;
    return *this;
  }

  /// adds two values, returning type is type of lhs
  template <typename _rhs_type, typename _rhs_ratio>
    requires RatioLike<_rhs_ratio>
  constexpr unit_t operator+(
      const unit_t<_symbol, _exponent, _rhs_type, _rhs_ratio> &rhs) const {

    static_assert(
        SI_ENABLE_IMPLICIT_RATIO_CONVERSION ||
            std::ratio_equal_v<ratio, _rhs_ratio>,
        "Implicit ratio conversion disabled, convert before adding values");

    return unit_t{
        value() +
        unit_cast<unit_t<_symbol, _exponent, _type, _ratio>>(rhs).value()};
  }

  /// add-assign value of the same unit
  constexpr unit_t &operator+=(const unit_t &rhs) {
    value_ += rhs.value();
    return *this;
  }

  /// add value of the same type but possibly different ratio
  template <typename _rhs_type, typename _rhs_ratio>
    requires (!std::ratio_equal_v<_rhs_ratio, _ratio> && RatioLike<_rhs_ratio>)
  constexpr unit_t &
  operator+=(const unit_t<_symbol, _exponent, _rhs_type, _rhs_ratio> &rhs) {

    static_assert(
        SI_ENABLE_IMPLICIT_RATIO_CONVERSION ||
            std::ratio_equal_v<ratio, _rhs_ratio>,
        "Implicit ratio conversion disabled, convert before adding values");

    value_ += unit_cast<unit_t<_symbol, _exponent, _type, _ratio>>(rhs).value();

    return *this;
  }

  /// subtracts two values, returning type is type of lhs
  template <typename _rhs_type, typename _rhs_ratio>
    requires RatioLike<_rhs_ratio>
  constexpr unit_t operator-(
      const unit_t<_symbol, _exponent, _rhs_type, _rhs_ratio> &rhs) const {

    static_assert(
        SI_ENABLE_IMPLICIT_RATIO_CONVERSION ||
            std::ratio_equal_v<ratio, _rhs_ratio>,
        "Implicit ratio conversion disabled, convert before subtracting");

    return unit_t{
        value() +
        -unit_cast<unit_t<_symbol, _exponent, _type, _ratio>>(rhs).value()};
  }

  /// Subtract-assign value of the same unit
  constexpr unit_t &operator-=(const unit_t &rhs) {
    value_ -= rhs.value();
    return *this;
  }

  /// subtract value of the same type but possibly different ratio
  template <typename _rhs_type, typename _rhs_ratio>
    requires (!std::ratio_equal_v<_rhs_ratio, _ratio> && RatioLike<_rhs_ratio>)
  constexpr unit_t &
  operator-=(const unit_t<_symbol, _exponent, _type, _rhs_ratio> &rhs) {

    static_assert(
        SI_ENABLE_IMPLICIT_RATIO_CONVERSION ||
            std::ratio_equal_v<ratio, _rhs_ratio>,
        "Implicit ratio conversion disabled, convert before adding values");

    value_ -= unit_cast<unit_t<_symbol, _exponent, _type, _ratio>>(rhs).value();

    return *this;
  }

  /// negate operation
  constexpr unit_t operator-() const { return unit_t{-value_}; }

  /// increment by prefix ++
  unit_t &operator++() {
    ++value_;
    return *this;
  }
  /// increment by postfix ++
  unit_t operator++(int) {
    auto ret_val(*this);
    ++(*this);

    return ret_val;
  }

  /// decrement by prefix --
  unit_t &operator--() {
    --value_;
    return *this;
  }

  /// decrement by postfix --
  unit_t operator--(int) {
    auto ret_val(*this);
    --(*this);

    return ret_val;
  }

private:
  _type value_;
};

/// operator to divide scalar type by unit encapsulating the same type
/// template specialization handling integer types
/// @results unit with negative exponent
template <std::integral _type, char _symbol, typename _exponent, typename _rhs_type,
          typename _ratio>
  requires RatioLike<_exponent> && RatioLike<_ratio>
constexpr auto
operator/(const _type &lhs,
          const unit_t<_symbol, _exponent, _rhs_type, _ratio> &rhs) {
  static_assert(SI_ENABLE_IMPLICIT_RATIO_CONVERSION ||
                    std::ratio_equal<std::ratio<1>, _ratio>::value,
                "Implicit ratio conversion disabled, convert to ratio<1> "
                "before dividing");
  return unit_t<_symbol, std::ratio_multiply<std::ratio<-1>, _exponent>, _type,
                _ratio>{lhs / rhs.value()};
}

/// operator to divide scalar type by unit encapsulating the same type
/// template specialization for floating point types, to avoid possible loss
/// of precision when adjusting for ratio
/// @results unit with negative exponent
template <std::floating_point _type, char _symbol, typename _exponent, typename _rhs_type,
          typename _ratio>
  requires RatioLike<_exponent> && RatioLike<_ratio>
constexpr auto
operator/(const _type &lhs,
          const unit_t<_symbol, _exponent, _rhs_type, _ratio> &rhs) {
  static_assert(SI_ENABLE_IMPLICIT_RATIO_CONVERSION ||
                    std::ratio_equal_v<_ratio, std::ratio<1>>,
                "Implicit ratio conversion disabled, convert to ratio<1> "
                "before dividing");
  return unit_t<_symbol, std::ratio_multiply<std::ratio<-1>, _exponent>, _type,
                _ratio>{lhs / rhs.value()};
}

} // namespace SI::detail
