#pragma once

#include <map>

#include "pattern.hpp"

/**
Node in a Nondeterministic Finite Automaton (State Machine)
**/
struct NFANode
{
  const void* match = nullptr;
  std::map< PatternChar, NFANode* > next;
};
