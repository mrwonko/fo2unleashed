#include "signature.hpp"
#include "pattern/nfa.hpp"
#include "pattern/dfa.hpp"
#include "logger.hpp"

#include <set>
#include <utility>
#include <map>
#include <sstream>

bool findSignatures( const std::vector< Region >& regions, const std::vector< Signature >& signatures )
{
  Logger& logger = Logger::getSingleton();

  std::set< const Signature* > mandatory;
  NFA nfa;
  for( auto& sig : signatures )
  {
    nfa.insert( sig.pattern.data(), sig.pattern.size(), &sig );
    if( sig.mandatory )
    {
      mandatory.insert( &sig );
    }
  }

  try
  {
    DFA dfa( std::move( nfa ) );

    std::map< const Signature*, char* > matches;

    // Search through all regions
    for( auto& region : regions )
    {
      logger.verbose( "Searching region (0x", std::hex, reinterpret_cast< const void* >( region.begin() ), ", 0x", region.size(), std::dec, ")." );
      if( !dfa.run( region.begin(), region.size(), [ &mandatory, &matches, &region ]( ptrdiff_t offset, const void* opaque )
      {
        const Signature* sig = reinterpret_cast< const Signature* >( opaque );
        mandatory.erase( sig );
        char* position = region.begin() + offset;
        if( !matches.emplace( sig, position ).second )
        {
          Logger::getSingleton().error( "Found multiple matches for code signature ", sig->name, "!" );
          return false;
        }
        Logger::getSingleton().verbose( "Found signature ", sig->name, " at 0x", std::hex, reinterpret_cast< void* >( position ), std::dec );
        return true;
      } ) )
      {
        return false;
      }
    }

    // Did we find all mandatory entries?
    if( !mandatory.empty() )
    {
      std::stringstream names;
      bool first = true;
      for( auto sig : mandatory )
      {
        if( !first ) names << ", ";
        first = false;
        names << sig->name;
      }
      logger.error( "Did not find code signature in FlatOut2.exe: ", names.str(), "!" );
      return false;
    }

    // Report findings
    for( auto& match : matches )
    {
      if( !match.first->onFound( match.second ) )
      {
        logger.error( "Could not apply code patch ", match.first->name, "!" );
        return false;
      }
      logger.verbose( "Applied code patch ", match.first->name, " at location 0x", std::hex, reinterpret_cast< const void* >( match.second ), std::dec );
    }

    return true;
  }
  catch( DuplicateEntryException )
  {
    logger.error( "Duplicate function signature pattern registration!" );
    return false;
  }
}

