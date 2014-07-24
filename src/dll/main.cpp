#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <Psapi.h>

#include <vector>
#include <string>

#include "logger.hpp"

bool enumModules()
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
    MODULEINFO info;
    if( !GetModuleInformation( curProcess, module, &info, sizeof( info ) ) )
    {
      logger.warning( "Failed to retrieve information on module ", name, "! (", GetLastError(), ")" );
      continue;
    }
    logger.info( name, " starts at address 0x", std::hex, info.lpBaseOfDll, std::dec, " and has a size of ", info.SizeOfImage );
  }
  return true;
}

HINSTANCE g_dll;

BOOL APIENTRY DllMain( HINSTANCE hinstDLL, DWORD  fdwReason, LPVOID lpvReserved )
{
  if( fdwReason != DLL_PROCESS_ATTACH ) return TRUE;

  Logger& logger = Logger::getSingleton();

  g_dll = hinstDLL;

  logger.info( "dll has been attached to process." );

  // TODO: Load & apply config

  if( !enumModules() ) return FALSE;

  MessageBoxA( nullptr, "DLL Says hi!", "Hi!", MB_OK | MB_ICONINFORMATION );

  return TRUE;
}
