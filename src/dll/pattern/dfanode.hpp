#pragma once

#include <cstring>

/**
Node in a Deterministic Finite Automaton (State Machine)
**/
struct DFANode
{
  const void* match = nullptr;
  DFANode* next[ 256 ];

  DFANode( const void* match )
    : match( match )
  {
  }
};
