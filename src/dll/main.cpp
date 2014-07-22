#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

HINSTANCE g_dll;

BOOL APIENTRY DllMain( HINSTANCE hinstDLL, DWORD  fdwReason, LPVOID lpvReserved )
{
  if( fdwReason != DLL_PROCESS_ATTACH ) return TRUE;
  g_dll = hinstDLL;

  MessageBoxA( nullptr, "DLL Says hi!", "Hi!", MB_OK | MB_ICONINFORMATION );

  return TRUE;
}
