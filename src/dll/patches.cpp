#include "patches.hpp"
#include "signature.hpp"
#include "addresses.hpp"
#include "logger.hpp"

#include <cstring>
#include <cstdint>

static std::vector< Signature > s_signatures =
{
  {
    "filesystem mount",
    true,
    { 0x83, 0xF8, 0x01     // 00 CMP EAX, 1
    , 0x75, 0x09           // 03 JNE +09
    , 0x5E                 // 05 POP ESI
    , 0x5D                 // 06 POP EBP
    , 0x83, 0xC8, 0xFF     // 07 OR EAX,FFFFFFFF
    , 0x5B                 // 10 POP EBX
    , 0xC2, 0x10, 0x00     // 11 RETN 10
    , 0x57                 // 14 PUSH EDI
    , 0xE8, -1, -1, -1, -1 // 15 CALL ...
    , 0xBE, -1, -1, -1, -1 // 20 MOV ESI, "filesystem"
    , 0xE8, -1, -1, -1, -1 // 25 CALL mountFromFileList, offset relative to next instruction
    , 0xBE, -1, -1, -1, -1 // 30 MOV ESI, "patch"
    , 0xE8, -1, -1, -1, -1 // 35 CALL mountFromFileList
    , 0x68, -1, -1, -1, -1 // 40 PUSH "-binarydb"
    , 0x53                 // 45 PUSH EBX
    , 0x33, 0xFF           // 46 XOR EDI, EDI
    },
    []( char* location )
    {
      const char* filesystem = *reinterpret_cast< const char** >( location + 21 );
      if( strncmp( filesystem, "filesystem", 11 ) != 0 )
      {
        Logger::getSingleton().error( "Reference does not point to \"filesystem\" string!" );
        return false;
      }
      std::int32_t offset = *reinterpret_cast< const std::int32_t* >( location + 26 );
      const char* nextInstr = location + 30;
      FO2_detail_mountFromFileList = reinterpret_cast< void( *)() >( nextInstr + offset );
      return true;
    }
  }
};

bool applyPatches( const std::vector< Region >& regions )
{
  Logger::getSingleton().verbose( "Starting signature search." );
  bool success = findSignatures( regions, s_signatures );
  Logger::getSingleton().verbose( "Signature search finished." );
  return success;
}
