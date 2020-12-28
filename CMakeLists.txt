# Basic parameters; check that these match your project / environment
cmake_minimum_required(VERSION 3.9)
project(32blit-misc)

set(32BLIT_PATH "../" CACHE PATH "Path to 32blit.cmake")
set(PROJECT_DISTRIBS LICENSE README.md)

# Build configuration; approach this with caution!
if(MSVC)
  add_compile_options("/W4" "/wd4244" "/wd4324")
else()
  add_compile_options("-Wall" "-Wextra" "-Wdouble-promotion")
endif()
if(NOT EXISTS ${32BLIT_PATH}/32blit.cmake)
  message(FATAL_ERROR "Define location of 32Blit API with -D32BLIT_PATH=<path to 32blit.cmake>")
endif()
include (${32BLIT_PATH}/32blit.cmake)
install (FILES ${PROJECT_DISTRIBS} DESTINATION .)

add_subdirectory(launcher-test)
add_subdirectory(screen-mode)
add_subdirectory(sd-test)