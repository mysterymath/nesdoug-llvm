cmake_minimum_required(VERSION 3.18)

set(LLVM_MOS_PLATFORM nes-nrom)
find_package(llvm-mos-sdk REQUIRED)

project(21_sprite_zero LANGUAGES C ASM)

set(CMAKE_EXECUTABLE_SUFFIX .nes)

add_executable(sprite_zero
  sprite_zero.c
  Sprites.c
  chr-rom.s
)
target_include_directories(sprite_zero PRIVATE .)
target_link_libraries(sprite_zero nesdoug neslib famitone2 rooms music)
set_property(SOURCE chr-rom.s PROPERTY
  OBJECT_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/sprite_zero.chr)

add_subdirectory(BG)
add_subdirectory(MUSIC)
