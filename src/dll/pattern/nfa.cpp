#include "nfa.hpp"

#include <utility>
#include <cassert>


NFA::NFA()
{
  m_nodes.emplace_back( new NFANode() );
}

NFA::NFA( NFA&& rhs )
: m_nodes( std::move( rhs.m_nodes ) )
{
}

NFA& NFA::operator=( NFA&& rhs )
{
  m_nodes = std::move( rhs.m_nodes );
  return *this;
}

bool NFA::insert( const PatternChar* data, size_t length, const void* id )
{
  return insert( root(), data, data + length, length, id );
}

bool NFA::insert( NFANode* node, const PatternChar* it, const PatternChar* end, size_t length, const void* id )
{
  // Match for this node
  if( it == end )
  {
    return node->matches.emplace( id, length ).second;
  }
  // Match for child
  auto entry = node->next.find( *it );
  if( entry != node->next.end() )
  {
    // existing child
    return insert( entry->second, it + 1, end, length, id );
  }
  // new child
  m_nodes.emplace_back( new NFANode() );
  auto newNode = m_nodes.back().get();
  bool success = insert( newNode, it + 1, end, length, id ); // not super efficient, but same big-O as most efficient solution
  assert( success );
  node->next.emplace( *it, newNode );
  return true;
}
