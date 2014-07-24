/**

This program launches Flatout 2 and injects the FO2Unleashed.dll into it.

The goal is to inject it before any code is executed so the startup can be hooked. To this end, the process is created suspended.
However, at that point some necessary initialization is reportedly not done (see http://opcode0x90.wordpress.com/2011/01/15/injecting-dll-into-process-on-load/), so we replace the program code with an infinite loop and resume execution until that loop is hit, at which point we inject the dll, restore the code and go on.

**/

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <string>
#include <algorithm>
#include <vector>
#include <iostream>
#include <fstream>
#include <functional>

enum
{
  TRIES = 50,
  MS_BETWEEN_TRIES = 100,
};

//    Command Line Argument Handling

static std::string escape( const std::string& str )
{
  std::string result;
  std::string backslashes;
  for( char c : str )
  {
    if( c == '\\' )
    {
      backslashes += '\\';
    }
    else
    {
      result += backslashes;
      // only escape backslashes if they are in front of a quote
      if( c == '"' )
      {
        result += backslashes;
        result += '\\';
      }
      result += c;
      backslashes = "";
    }
  }
  return result;
}

static std::string concatArguments( const int argc, const char** argv )
{
  std::string commandLine;
  for( const char ** arg = argv; arg != argv + argc; ++arg )
  {
    if( arg != argv ) commandLine += ' ';
    std::string escapedArg = escape( *arg );
    if( escapedArg.find( ' ' ) == std::string::npos )
    {
      commandLine += escapedArg;
    }
    else
    {
      commandLine += '"';
      commandLine += escapedArg;
      commandLine += '"';
    }
  }
  return commandLine;
}

//    Injection

/**
Retrieves a PE's entry point.
Returns nullptr on error.
**/
static void* readEntryPoint( const std::string& filename )
{
  std::ifstream file( filename.c_str(), std::ios::binary );
  if( file.fail() )
  {
    std::cerr << "Failed to open " << filename << "!" << std::endl;
    return nullptr;
  }

  {
    IMAGE_DOS_HEADER dosHeader;
    file.read( reinterpret_cast< char* >( &dosHeader ), sizeof( dosHeader ) );
    if( file.fail() )
    {
      std::cerr << "Failed to read header from " << filename << "!" << std::endl;
      return nullptr;
    }

    if( dosHeader.e_magic != IMAGE_DOS_SIGNATURE )
    {
      std::cerr << filename << " does not start with a DOS header!" << std::endl;
    }

    file.seekg( dosHeader.e_lfanew );
    if( file.fail() || file.eof() )
    {
      std::cerr << "Failed to go to PE header in " << filename << "!" << std::endl;
      return nullptr;
    }
  }

  {
    IMAGE_NT_HEADERS ntHeaders;
    file.read( reinterpret_cast< char* >( &ntHeaders ), sizeof( ntHeaders ) );
    if( file.fail() )
    {
      std::cerr << "Failed to read PE header from " << filename << "!" << std::endl;
      return nullptr;
    }

    if( ntHeaders.Signature != IMAGE_NT_SIGNATURE )
    {
      std::cerr << filename << " does not contain a valid PE header!" << std::endl;
      return nullptr;
    }

    void* result = reinterpret_cast< void* >( ntHeaders.OptionalHeader.ImageBase + ntHeaders.OptionalHeader.AddressOfEntryPoint );

    if( !result )
    {
      std::cerr << filename << " has entryPoint 0, which isn't handled properly - complain and I'll fix it, but I don't think this will ever happen." << std::endl;
    }

    return result;
  }
}

// Display error message and windows error code
void errorWithCode( const char* message )
{
  std::cerr << message << " (" << GetLastError() << ")" << std::endl;
};

static bool injectDLL( const std::string& dllName, const PROCESS_INFORMATION& info )
{
  void* remotePathMem = VirtualAllocEx( info.hProcess, nullptr, dllName.size() + 1, MEM_COMMIT, PAGE_READWRITE );
  if( !remotePathMem )
  {
    errorWithCode( "Failed to allocate memory for dll name!" );
    return false;
  }
  std::function< void( const char* ) > cleanup = [ &info, remotePathMem ]( const char* message )
  {
    if( message ) errorWithCode( message );
    VirtualFreeEx( info.hProcess, remotePathMem, 0, MEM_RELEASE );
  };

  SIZE_T bytesWritten;
  if( !WriteProcessMemory( info.hProcess, remotePathMem, dllName.c_str(), dllName.size() + 1, &bytesWritten ) || bytesWritten != dllName.size() + 1 )
  {
    cleanup( "Failed to write dll name to memory!" );
    return false;
  }

  // Retrieve LoadLibrary address. Since Kernel32 is always loaded and always has the same address space, we can just use our address.
  HMODULE hKernel32 = GetModuleHandleA( "Kernel32" );
  LPTHREAD_START_ROUTINE loadLibraryAddress = reinterpret_cast< LPTHREAD_START_ROUTINE >( GetProcAddress( hKernel32, "LoadLibraryA" ) );

  HANDLE libraryThread = CreateRemoteThread(
    info.hProcess,  // process
    nullptr, // attributes
    0, // stack size
    loadLibraryAddress, // Remote function address
    remotePathMem, // attribute
    0, // creation flags
    nullptr // thread id
    );
  if( !libraryThread )
  {
    cleanup( "Failed to create thread in FlatOut2 process!" );
    return false;
  }
  cleanup = [ cleanup, libraryThread ]( const char * message )
  {
    cleanup( message );
    CloseHandle( libraryThread );
  };

  auto waitResult = WaitForSingleObject( libraryThread, INFINITE );
  if( waitResult != WAIT_OBJECT_0 )
  {
    std::cerr << "Warning: Unexpected return code from waiting on DLL thread: " << waitResult << std::endl;
  }

  DWORD loadedLibrary;
  if( !GetExitCodeThread( libraryThread, &loadedLibrary ) )
  {
    cleanup( "Failed to retrieve LoadLibrary exit code from FlatOut2 process!" );
    return false;
  }
  if( !loadedLibrary )
  {
    std::cerr << "FlatOut2 process failed to load " << dllName << "!" << std::endl;
    cleanup( nullptr );
    return false;
  }
  cleanup( nullptr );
  return true;
}

static void destroyProcess( const PROCESS_INFORMATION& info )
{
  TerminateProcess( info.hProcess, -1 );
}

static bool createProcess( const std::string& processName, const std::string& commandLine, const std::string& dllName )
{

  // command line parameter must not be const, so let's copy it to a temp buffer
  std::vector< char > commandLineWritable( commandLine.size() + 1 );
  std::copy( commandLine.begin(), commandLine.end(), commandLineWritable.begin() );
  commandLineWritable[ commandLine.size() ] = '\0';

  std::cout << commandLineWritable.data() << std::endl;

  // Create suspended process
  STARTUPINFOA startupInfo{};
  PROCESS_INFORMATION info;
  startupInfo.cb = sizeof( startupInfo );
  if( !CreateProcessA(
    processName.c_str(),
    commandLineWritable.data(),
    nullptr, // process attributes
    nullptr, // thread attributes
    FALSE, // inherit handles
    CREATE_SUSPENDED, // creation flags
    nullptr, // environment
    nullptr, // current directory
    &startupInfo,
    &info
    ) )
  {
    errorWithCode( "Failed to create process!" );
    return false;
  }

  // Function to report and clean up in case of error, updated as required
  const auto failureCleanup = [ &info ]( const char * message )
  {
    destroyProcess( info );
    if( message ) errorWithCode( message );
  };

  // Retrieve entry point
  void* entryPoint = readEntryPoint( processName );
  if( !entryPoint )
  {
    failureCleanup( nullptr ); // message already displayed by readEntryPoint()
    return false;
  }

  // Change memory protection on entry point so we can change it
  DWORD oldProtect;
  if( !VirtualProtectEx( info.hProcess, entryPoint, 2, PAGE_EXECUTE_READWRITE, &oldProtect ) )
  {
    failureCleanup( "Failed to acquire write rights in FlatOut2 code!" );
    return false;
  }

  // Function to reset the protection to its previous state
  const auto resetProtection = [ oldProtect, entryPoint, &info ]()
  {
    DWORD ignored;
    if( !VirtualProtectEx( info.hProcess, entryPoint, 2, oldProtect, &ignored ) )
    {
      errorWithCode( "Failed to reset write protection in FlatOut2 code!" );
      return false;
    }
    return true;
  };

  // Read previous code at entry point
  static const unsigned char newCode[]{0xEB, 0xFE}; // jmp $-2 ; infinite loop

  unsigned char previousCode[ sizeof( newCode ) ];
  {
    SIZE_T bytesRead;
    if( !ReadProcessMemory( info.hProcess, entryPoint, previousCode, sizeof( newCode ), &bytesRead ) || bytesRead != sizeof( newCode ) )
    {
      failureCleanup( "Failed to read from FlatOut2 code!" );
      return false;
    }
  }

  // Overwrite code at entry point with an infinite loop
  {
    SIZE_T bytesWritten;
    if( !WriteProcessMemory( info.hProcess, entryPoint, newCode, sizeof( newCode ), &bytesWritten ) || bytesWritten != sizeof( newCode ) )
    {
      failureCleanup( "Failed to overwrite FlatOut2 code!" );
      return false;
    }
  }

  // Resume process and wait for it to reach the entry point
  if( ResumeThread( info.hThread ) == -1 )
  {
    failureCleanup( "Failed to start FlatOut2 process!" );
    return false;
  }

  {
    CONTEXT context{};
    for( int i = 0; i < TRIES && reinterpret_cast< void* >( context.Eip ) != entryPoint; ++i )
    {
      Sleep( MS_BETWEEN_TRIES );
      context.ContextFlags = CONTEXT_CONTROL;
      if( !GetThreadContext( info.hThread, &context ) )
      {
        failureCleanup( "Failed to FlatOut2 thread context!" );
        return false;
      }
    }
    if( reinterpret_cast< void* >( context.Eip ) != entryPoint )
    {
      failureCleanup( "FlatOut2 process did not reach alleged entry point within time!" );
      return false;
    }
  }

  // Suspend process
  if( SuspendThread( info.hThread ) == -1 )
  {
    failureCleanup( "Failed to suspend FlatOut2 process!" );
    return false;
  }

  // Write back original code
  {
    SIZE_T bytesWritten;
    if( !WriteProcessMemory( info.hProcess, entryPoint, previousCode, sizeof( newCode ), &bytesWritten ) || bytesWritten != sizeof( newCode ) )
    {
      failureCleanup( "Failed to write back original FlatOut2 code!" );
      return false;
    }
  }

  // Reset write protection
  if( !resetProtection() )
  {
    failureCleanup( nullptr );
    return false;
  }

  // TODO: in Steam version, delay injection until code has been decrypted/modified

  // Inject DLL
  if( !injectDLL( dllName, info ) )
  {
    failureCleanup( nullptr );
    return false;
  }

  // Resume FlatOut2 thread
  if( ResumeThread( info.hThread ) == -1 )
  {
    failureCleanup( "Failed to resume FlatOut2 process!" );
    return false;
  }

  return true;
}

int main( const int argc, const char** argv )
{
  std::vector< char > buf( 1024 );
  buf.resize( GetCurrentDirectoryA( buf.size(), buf.data() ) );
  std::string currentDirectory( buf.data(), buf.size() );
  if( !currentDirectory.empty() )
  {
    currentDirectory += '\\';
  }
  const std::string processName = "FlatOut2.exe";
  const std::string dllName = currentDirectory + "FO2Unleashed.dll";
  //const std::string dllName = "S:\\Games\\FlatOut2_modding\\FO2Unleashed.dll";

  std::vector< const char * > childArgs{ processName.c_str() };
  std::copy( argv + 1, argv + argc, std::back_inserter( childArgs ) );

  if( !createProcess( processName, concatArguments( childArgs.size(), childArgs.data() ), dllName ) )
  {
    return 1;
  }
  return 0;
}
