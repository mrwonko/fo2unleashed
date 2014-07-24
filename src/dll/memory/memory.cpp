#include "memory.hpp"
#include "../logger.hpp"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <Psapi.h>

#include <string>
#include <cctype>
#include <algorithm>
#include <cstddef>
#include <stdexcept>

static bool findFlatout2Module( char*& out_begin, std::ptrdiff_t& out_size )
{
  Logger& logger = Logger::getSingleton();

  HANDLE curProcess = GetCurrentProcess();

  DWORD numModules;
  if( !EnumProcessModules( curProcess, nullptr, 0, &numModules ) )
  {
    logger.error( "Failed to retrieve module count! (", GetLastError(), ")" );
    return false;
  }
  numModules /= sizeof( HMODULE );

  std::vector< HMODULE > modules( numModules );
  if( !EnumProcessModules( curProcess, modules.data(), sizeof( modules[ 0 ] ) * modules.size(), &numModules ) )
  {
    logger.error( "Failed to retrieve module list! (", GetLastError(), ")" );
    return false;
  }
  numModules /= sizeof( HMODULE );

  if( numModules < modules.size() )
  {
    modules.resize( numModules );
  }

  for( HMODULE module : modules )
  {
    std::string name;
    {
      std::vector< char > nameBuf( 1024 );
      if( !GetModuleBaseNameA( curProcess, module, nameBuf.data(), nameBuf.size() ) )
      {
        logger.warning( "Failed to retrieve name of module ", module, "! (", GetLastError(), ")" );
        continue;
      }
      name = nameBuf.data();
    }
    std::transform( name.begin(), name.end(), name.begin(), std::tolower );
    if( name != "flatout2.exe" )
    {
      continue;
    }
    MODULEINFO info;
    if( !GetModuleInformation( curProcess, module, &info, sizeof( info ) ) )
    {
      logger.error( "Failed to retrieve information on module ", name, "! (", GetLastError(), ")" );
      return false;
    }
    out_begin = reinterpret_cast< char* >( info.lpBaseOfDll );
    out_size = info.SizeOfImage;
    logger.info( "Found FlatOut2.exe module of size 0x", std::hex, out_size, " at 0x", reinterpret_cast< void* >( out_begin ), std::dec );
    return true;
  }
  logger.error( "Failed to find module FlatOut2.exe!" );
  return false;
}

bool getExecutableRegions( std::vector< Region >& out_regions )
{
  Logger& logger = Logger::getSingleton();
  out_regions.clear();

  char* start;
  std::ptrdiff_t size;
  if( !findFlatout2Module( start, size ) )
  {
    return false;
  }

  while( size > 0 )
  {
    MEMORY_BASIC_INFORMATION info = {};
    if( !VirtualQuery( start, &info, sizeof( info ) ) )
    {
      logger.error( "Failed to query page information of address ", start, "! (", GetLastError(), ")" );
      out_regions.clear();
      return false;
    }
    if( info.Protect & ( PAGE_EXECUTE | PAGE_EXECUTE_READ | PAGE_EXECUTE_READWRITE | PAGE_EXECUTE_WRITECOPY ) )
    {
      logger.info( "Found executable page(s) of size 0x", std::hex, info.RegionSize, " at 0x", info.BaseAddress, std::dec );
      auto modifiers = info.Protect & ~0xFF;
      try
      {
        out_regions.emplace_back( info.BaseAddress, info.RegionSize, PAGE_EXECUTE_READWRITE | modifiers );
      }
      catch( std::runtime_error& e )
      {
        logger.error( e.what() );
        out_regions.clear();
        return false;
      }
    }

    std::ptrdiff_t startOffset = start - reinterpret_cast< char* >( info.BaseAddress );
    std::ptrdiff_t regionSize = info.RegionSize - startOffset;
    size -= regionSize;
    start += regionSize;
  }
  return true;
}
