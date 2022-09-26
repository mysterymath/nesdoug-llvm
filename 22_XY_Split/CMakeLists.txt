cmake_minimum_required(VERSION 3.18)

set(LLVM_MOS_PLATFORM nes-nrom)
find_package(llvm-mos-sdk REQUIRED)

project(22_xy_split LANGUAGES C ASM)

set(CMAKE_EXECUTABLE_SUFFIX .nes)

add_executable(xy_split
  xy_split.c
  Sprites.c
  chr-rom.s
)
target_include_directories(xy_split PRIVATE .)
target_link_libraries(xy_split nesdoug neslib famitone2 rooms music)
set_property(SOURCE chr-rom.s PROPERTY
  OBJECT_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/xy_split.chr)

add_subdirectory(BG)
add_subdirectory(MUSIC)
