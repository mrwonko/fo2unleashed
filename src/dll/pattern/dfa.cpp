#include "dfa.hpp"
#include "nfa.hpp"
#include "pattern.hpp"

#include <map>
#include <set>
#include <utility>

DFA::DFA( const NFA& nfa )
{
  typedef std::set< const NFANode* > State;
  const NFANode* root = nfa.root();
  std::map< State, DFANode* > unhandledNodes{ std::make_pair( State{ root }, mkNode( root->match ) ) };
  std::map< State, DFANode* > allNodes( unhandledNodes );
  // Function to retrieve a node, or create it if necessary
  const auto getNode = [ &allNodes, &unhandledNodes, this ]( const State& state )
  {
    const auto entry = allNodes.find( state );
    if( entry == allNodes.end() )
    {
      const void* match = nullptr;
      for( const NFANode* node : state )
      {
        if( node->match )
        {
          // Duplicates?
          if( match ) throw DuplicateEntryException();
          match = node->match;
        }
      }
      DFANode* result = mkNode( match );
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
}

DFANode* DFA::mkNode( const void * match )
{
  m_nodes.emplace_back( new DFANode( match ) );
  return m_nodes.back().get();
}

void DFA::run( const char* data, const size_t length, std::function< void( const char *, const void* ) > onMatch ) const
{
  const DFANode* curNode = root();
  const char * it = data;
  const char * const end = data + length;
  while( true )
  {
    if( curNode->match ) onMatch( reinterpret_cast< const char * >( it ), curNode->match );
    if( it == end ) break;
    curNode = curNode->next[ *reinterpret_cast< const unsigned char * >( it ) ];
    ++it;
  }
}
