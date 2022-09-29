cmake_minimum_required(VERSION 3.18)

set(LLVM_MOS_PLATFORM nes-nrom)
find_package(llvm-mos-sdk REQUIRED)

project(25_full_game LANGUAGES C ASM)

set(CMAKE_EXECUTABLE_SUFFIX .nes)

add_executable(full_game
  full_game.c
  level_data.c
  palettes.c
  Sprites.c
  chr-rom.s
  NES_ST/title.c
)
target_include_directories(full_game PRIVATE .)
target_link_libraries(full_game nesdoug neslib famitone2 levels music)
set_property(SOURCE chr-rom.s PROPERTY
  OBJECT_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/full_game.chr)

add_subdirectory(BG)
add_subdirectory(MUSIC)
