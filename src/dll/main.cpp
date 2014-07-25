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
  if( !getExecutableRegions( regions ) )
  {
    MessageBoxA( nullptr, "Failed to identify module FlatOut2.exe! Refer to fo2unleashed.log for additional information.", "FlatOut 2 Unleashed - Error", MB_OK | MB_ICONERROR );
    return FALSE;
  }

  if( !applyPatches( regions ) )
  {
    MessageBoxA( nullptr, "Failed to apply memory patches! Refer to fo2unleashed.log for additional information.", "FlatOut 2 Unleashed - Error", MB_OK | MB_ICONERROR );
    return FALSE;
  }

  return TRUE;
}
