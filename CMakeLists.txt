cmake_minimum_required(VERSION 3.16)
project(minilab_reaper_surface LANGUAGES CXX)

include(CTest)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(REAPER_SDK_DIR "${CMAKE_CURRENT_SOURCE_DIR}/sdk/reaper-sdk/sdk" CACHE PATH "Path to the REAPER SDK headers")

add_executable(minilab_reaper_surface_tests
  tests/test_minilab_reaper_surface.cpp
  src/minilab_reaper_surface.cpp
  src/minilab_reaper_extension.cpp
  src/minilab_midi_handler.cpp
  src/minilab_reaper_plugin.cpp
)

target_include_directories(minilab_reaper_surface_tests PRIVATE src ${REAPER_SDK_DIR})
add_test(NAME minilab_reaper_surface_tests COMMAND minilab_reaper_surface_tests)

add_library(minilab_reaper_plugin SHARED
  src/minilab_reaper_plugin.cpp
  src/minilab_reaper_extension.cpp
  src/minilab_reaper_surface.cpp
  src/minilab_midi_handler.cpp
  sdk/WDL/WDL/swell/swell-modstub-generic.cpp
)

target_include_directories(minilab_reaper_plugin PRIVATE src ${REAPER_SDK_DIR})
target_compile_definitions(minilab_reaper_plugin PRIVATE SWELL_PROVIDED_BY_APP)
target_link_libraries(minilab_reaper_plugin PRIVATE ${CMAKE_DL_LIBS})
set_target_properties(minilab_reaper_plugin PROPERTIES
  PREFIX "reaper_"
  OUTPUT_NAME "csurf_minilab"
  POSITION_INDEPENDENT_CODE ON)

install(TARGETS minilab_reaper_plugin LIBRARY DESTINATION /opt/REAPER/Plugins)
