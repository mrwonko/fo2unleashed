#pragma once

#include <map>

#include "pattern.hpp"

/**
Node in a Nondeterministic Finite Automaton (State Machine)
**/
struct NFANode
{
  std::map< const void*, size_t > matches; ///< matches by length
  std::map< PatternChar, NFANode* > next;
};
