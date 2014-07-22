#pragma once

#include <cstdint>

typedef std::int16_t PatternChar;
enum {
  PATTERN_WILDCARD = -1,
  PATTERN_MIN = 0,
  PATTERN_MAX = ( 1 << 8 ) - 1,
};
