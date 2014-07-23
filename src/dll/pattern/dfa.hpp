#pragma once

#include "dfanode.hpp"

#include <stdexcept>
#include <vector>
#include <memory>
#include <functional>

class DuplicateEntryException : public std::logic_error
{
public:
  DuplicateEntryException() : std::logic_error( "Duplicate/overlapping entry in NFA" ) {}
};

class NFA;
struct DFANode;

class DFA
{
public:
  /// @throws DuplicateEntryException
  DFA( NFA&& nfa );

  /// @param onMatch function to call on match, first parameter is offset to start of match.
  void run( const char* data, const size_t length, std::function< void( ptrdiff_t, const void* ) > onMatch ) const;

private:
  DFANode* mkNode( std::map< const void*, size_t >&& matches );
  const DFANode* root() const { return m_nodes.front().get(); }

private:
  std::vector< std::unique_ptr< DFANode > > m_nodes;
};
