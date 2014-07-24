#pragma once

#include <vector>
#include <functional>

#include "pattern/pattern.hpp"
#include "memory/region.hpp"

struct Signature
{
  const char* name;
  bool mandatory;
  std::vector< PatternChar > pattern;
  std::function< bool( char* ) > onFound;
};

bool findSignatures( const std::vector< Region >& regions, const std::vector< Signature >& signatures );
