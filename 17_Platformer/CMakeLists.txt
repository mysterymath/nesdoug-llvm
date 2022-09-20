cmake_minimum_required(VERSION 3.18)

set(LLVM_MOS_PLATFORM nes-nrom)
find_package(llvm-mos-sdk REQUIRED)

project(17_platformer LANGUAGES C ASM)

set(CMAKE_EXECUTABLE_SUFFIX .nes)

add_executable(platformer
  platformer.c
  Sprites.c
  chr-rom.s
)
target_include_directories(platformer PRIVATE .)
target_link_libraries(platformer nesdoug neslib rooms)
set_property(SOURCE chr-rom.s PROPERTY
  OBJECT_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/platformer.chr)

add_subdirectory(BG)
