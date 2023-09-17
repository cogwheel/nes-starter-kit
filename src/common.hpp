#pragma once

// This file contains miscellaneous constants, helpers, etc. that are used
// throughout the project

namespace cog {

constexpr char kScreenWidth = 32;
constexpr char kScreenHeight = 30;
constexpr int kScreenSize = kScreenWidth * kScreenHeight;

constexpr char kPixelsPerTile = 8;

} // end namespace cog

#define UNUSED(var) ((void)(var))

// These aren't in llvm-mos <algoithm> header
//
// Note: these do not follow the standard, which expects const reference instead
// of value parameters. In testing (against SDK 6 or so), these alternatives
// produced better code.
namespace std {
template <typename T> constexpr T min(T const a, T const b) {
  return a < b ? a : b;
}

template <typename T> constexpr T max(T const a, T const b) {
  return b < a ? a : b;
}

template <typename T>
constexpr T clamp(T const val, T const minval, T const maxval) {
  return max(min(val, maxval), minval);
}
} // namespace std
