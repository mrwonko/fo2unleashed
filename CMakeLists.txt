cmake_minimum_required( VERSION 2.8.8 )

project( FO2Unleashed )

if( CMAKE_COMPILER_IS_GNUCXX )
	set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11" )
endif( CMAKE_COMPILER_IS_GNUCXX )

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
	${PatternFiles}
	)

add_executable( PatternTest
	src/test/pattern.cpp
	${PatternFiles}
	)

install( TARGETS FO2UnleashedLauncher FO2Unleashed RUNTIME DESTINATION bin )
