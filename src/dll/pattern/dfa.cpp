#include "dfa.hpp"
#include "nfa.hpp"
#include "pattern.hpp"

#include <map>
#include <set>
#include <utility>

DFA::DFA( NFA&& nfa )
{
  typedef std::set< NFANode* > State;
  NFANode* root = nfa.root();
  std::map< State, DFANode* > unhandledNodes{ std::make_pair( State{ root }, mkNode( std::move( root->matches ) ) ) };
  std::map< State, DFANode* > allNodes( unhandledNodes );
  // Function to retrieve a node, or create it if necessary
  const auto getNode = [ &allNodes, &unhandledNodes, this ]( const State& state )
  {
    const auto entry = allNodes.find( state );
    if( entry == allNodes.end() )
    {
      std::map< const void*, size_t > matches;
      const void* match = nullptr;
      for( const NFANode* node : state )
      {
        for( auto& match : node->matches )
        {
          if( !matches.emplace( match.first, match.second ).second )
          {
            throw DuplicateEntryException();
          }
        }
      }
      DFANode* result = mkNode( std::move( matches ) );
      unhandledNodes[ state ] = result;
      allNodes[ state ] = result;
      return result;
    }
    return entry->second;
  };
  while( !unhandledNodes.empty() )
  {
    auto unhandled = unhandledNodes.begin();
    const State& curState = unhandled->first;
    DFANode* curNode = unhandled->second;

    // Which nodes are accessible in any case?
    State nextWildcards{ root };
    for( const NFANode* node : curState )
    {
      auto entry = node->next.find( PATTERN_WILDCARD );
      if( entry != node->next.end() )
      {
        nextWildcards.insert( entry->second );
      }
    }

    // Which nodes are accessible via a certain letter?
    for( PatternChar c = PATTERN_MIN; c <= PATTERN_MAX; ++c )
    {
      State nextState( nextWildcards );
      for( const NFANode* node : curState )
      {
        auto entry = node->next.find( c );
        if( entry != node->next.end() )
        {
          nextState.insert( entry->second );
        }
      }
      curNode->next[ c ] = getNode( nextState );
    }
    unhandledNodes.erase( unhandled );
  }
  nfa.clear();
}

DFANode* DFA::mkNode( std::map< const void*, size_t >&& matches )
{
  m_nodes.emplace_back( new DFANode( std::move( matches ) ) );
  return m_nodes.back().get();
}

bool DFA::run( const unsigned char* data, const size_t length, std::function< bool( ptrdiff_t, const void* ) > onMatch ) const
{
  const DFANode* curNode = root();
  const unsigned char * it = data;
  const unsigned char * const end = data + length;
  while( true )
  {
    for( auto& match : curNode->matches )
    {
      if( !onMatch( it - match.second - data, match.first ) )
      {
        return false;
      }
    }
    if( it == end ) break;
    curNode = curNode->next[ *it ];
    ++it;
  }
  return true;
}
