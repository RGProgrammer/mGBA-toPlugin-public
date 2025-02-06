 cmake_minimum_required(VERSION 3.10)
project("mgba_plugin")
set(CMAKE_CXX_STANDARD 20)
set(LIBMGBA_ONLY ON)
set(M_CORE_GBA ON)
set(M_CORE_GB ON)
set(DISABLE_THREADING ON)
set(USE_DEBUGGERS OFF)
set(MGBA_STANDALONE ON)

if(DEFINED ANDROID_NDK)
  add_definitions(
        -DHAVE_STRTOF_L
        -D_STL_COMPILER_PREPROCESSOR)
endif()

include_directories("./mgba/include/mgba"
                      "./")
add_subdirectory("./mgba")
add_library( 
            mgba_plugin SHARED
            GameBoyAudio.h
            GameBoyAudio.cpp
            GameBoySave.h
            GameBoySave.cpp
            mgba_plugin.h
            mgba_plugin.cpp
          
)

target_link_libraries( mgba_plugin mgba)