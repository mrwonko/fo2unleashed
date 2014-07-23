#pragma once

#include <map>
#include <utility>

/**
Node in a Deterministic Finite Automaton (State Machine)
**/
struct DFANode
{
  std::map< const void*, size_t > matches;
  DFANode* next[ 256 ];

  DFANode()
    : next{}
  {
  };

  explicit DFANode( std::map< const void*, size_t >&& matches )
    : matches( std::move( matches ) )
    , next{}
  {
  }
};
