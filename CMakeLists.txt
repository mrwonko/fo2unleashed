cmake_minimum_required( VERSION 3.0 )

project( FO2Unleashed )
enable_language( ASM_MASM )

if( CMAKE_COMPILER_IS_GNUCXX )
	set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11" )
endif( CMAKE_COMPILER_IS_GNUCXX )

if( MSVC )
	# disable safe structured exceptions so we don't have to register a handler in the asm code
	set( CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /SAFESEH:NO" )
	set( CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SAFESEH:NO" )
endif( MSVC )

include_directories( "src" "include" )

add_executable( FO2UnleashedLauncher
	src/launcher/main.cpp
	)

set( PatternFiles
	src/dll/pattern/nfanode.hpp
	src/dll/pattern/nfa.cpp src/dll/pattern/nfa.hpp
	src/dll/pattern/dfanode.hpp
	src/dll/pattern/dfa.cpp src/dll/pattern/dfa.hpp
	src/dll/pattern/pattern.hpp
	)

add_library( FO2Unleashed SHARED
	src/dll/main.cpp
	src/dll/logger.hpp src/dll/logger.cpp
	${PatternFiles}
	)
target_compile_definitions( FO2Unleashed PUBLIC PSAPI_VERSION=1 ) # psapi v1 is backwards compatible with WinXP
target_link_libraries( FO2Unleashed Psapi.lib ) # psapi v1 uses psapi.lib, not kernel32.lib

add_executable( PatternTest
	src/test/pattern.cpp
	${PatternFiles}
	)

add_executable( AsmTest
	src/test/asmtest.cpp
	src/test/asm.asm
	)

install( TARGETS FO2UnleashedLauncher FO2Unleashed RUNTIME DESTINATION bin )
