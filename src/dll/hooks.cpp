#include "fo2api.hpp"
#include "logger.hpp"

extern "C"
{

  void hook_mount()
  {
    Logger::getSingleton().verbose( "hook_mount()" );
    // After loading files listed in "filesystem" and "patch", let's also load those listed in "mods.txt"
    FO2_mountFromFileList( "mods.txt" );
  }

}
