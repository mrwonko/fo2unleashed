#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include "logger.hpp"
#include "memory/memory.hpp"
#include "memory/region.hpp"
#include "patches.hpp"

#include <vector>

HINSTANCE g_dll;

BOOL APIENTRY DllMain( HINSTANCE hinstDLL, DWORD  fdwReason, LPVOID lpvReserved )
{
  if( fdwReason != DLL_PROCESS_ATTACH ) return TRUE;

  Logger& logger = Logger::getSingleton();

  g_dll = hinstDLL;

  logger.info( "dll has been attached to process." );

  // TODO: Load & apply config

  std::vector< Region > regions;
  if( !getExecutableRegions( regions ) ) return FALSE;

  if( !applyPatches( regions ) ) return FALSE;

  MessageBoxA( nullptr, "DLL Says hi!", "Hi!", MB_OK | MB_ICONINFORMATION );

  return TRUE;
}
