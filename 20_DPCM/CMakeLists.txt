cmake_minimum_required(VERSION 3.18)

set(LLVM_MOS_PLATFORM nes-nrom)
find_package(llvm-mos-sdk REQUIRED)

project(20_dpcm LANGUAGES C ASM)

set(CMAKE_EXECUTABLE_SUFFIX .nes)

add_executable(platformer4
  platformer4.c
  Sprites.c
  chr-rom.s
)
target_include_directories(platformer4 PRIVATE .)
target_link_libraries(platformer4 nesdoug neslib famitone2 rooms music)
set_property(SOURCE chr-rom.s PROPERTY
  OBJECT_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/platformer.chr)

add_subdirectory(BG)
add_subdirectory(MUSIC)
