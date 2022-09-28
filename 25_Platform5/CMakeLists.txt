cmake_minimum_required(VERSION 3.18)

set(LLVM_MOS_PLATFORM nes-nrom)
find_package(llvm-mos-sdk REQUIRED)

project(25_platform5 LANGUAGES C ASM)

set(CMAKE_EXECUTABLE_SUFFIX .nes)

add_executable(platformer5
  platformer5.c
  Sprites.c
  chr-rom.s
)
target_include_directories(platformer5 PRIVATE .)
target_link_libraries(platformer5 nesdoug neslib famitone2 rooms music)
set_property(SOURCE chr-rom.s PROPERTY
  OBJECT_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/platformer5.chr)

add_subdirectory(BG)
add_subdirectory(MUSIC)
