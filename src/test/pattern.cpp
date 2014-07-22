#include "dll/pattern/dfa.hpp"
#include "dll/pattern/nfa.hpp"

#include <iostream>
#include <string>
#include <vector>

int main( int argc, char** argv )
{
  std::cout << "Enter patterns to search for, * is wildcard, empty line to exit." << std::endl;
  std::vector< std::string > needles;
  std::string input;
  while( std::getline( std::cin, input ) && !input.empty() )
  {
    needles.push_back( input );
  }

  NFA nfa;
  for( const auto& needle : needles )
  {
    std::vector< PatternChar > buf;
    buf.reserve( needle.size() );
    for( auto c : needle )
    {
      buf.push_back( c == '*' ? PATTERN_WILDCARD : static_cast< unsigned char >( c ) );
    }
    nfa.insert( buf.data(), buf.size(), &needle );
  }
  try
  {
    DFA dfa( nfa );
    std::cout << "Enter text to search in." << std::endl;
    std::getline( std::cin, input );
    dfa.run( input.data(), input.size(), [ &input ]( const char * position, const void* match )
    {
      const std::string& needle = *reinterpret_cast< const std::string* >( match );
      std::cout << "Found \"" << needle << "\" at " << ( position - input.data() - needle.size() ) << std::endl;
    } );
  }
  catch( DuplicateEntryException )
  {
    std::cerr << "Overlapping needle ends!" << std::endl;
  }
}
