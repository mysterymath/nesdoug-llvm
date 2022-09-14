cmake_minimum_required(VERSION 3.18)

set(LLVM_MOS_PLATFORM nes-nrom)
find_package(llvm-mos-sdk REQUIRED)

project(12_metatiles LANGUAGES C ASM)

set(CMAKE_EXECUTABLE_SUFFIX .nes)

add_executable(metatiles
  metatiles.c
  Sprites.c
  chr-rom.s
  BG/Room1.c
)
target_include_directories(metatiles PRIVATE .)
target_link_libraries(metatiles nesdoug neslib)
set_property(SOURCE chr-rom.s PROPERTY
  OBJECT_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/metatiles.chr)
