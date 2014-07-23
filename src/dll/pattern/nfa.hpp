#pragma once

#include "pattern.hpp"
#include "nfanode.hpp"

#include <vector>
#include <memory>

class DFA;

class NFA
{
public:
  NFA();
  ~NFA() = default;
  NFA( const NFA& rhs ) = delete;
  NFA& operator=( const NFA& rhs ) = delete;
  NFA( NFA&& rhs );
  NFA& operator=( NFA&& rhs );

  bool insert( const PatternChar* data, size_t length, const void* id );

  const NFANode* root() const { return m_nodes.front().get(); }
  NFANode* root() { return m_nodes.front().get(); }
  void clear() { m_nodes.clear(); }

private:
  bool insert( NFANode* node, const PatternChar* it, const PatternChar* end, size_t length, const void* id );

private:
  std::vector< std::unique_ptr< NFANode > > m_nodes;
};
