cmake_minimum_required(VERSION 3.18)

set(LLVM_MOS_PLATFORM nes-nrom)
find_package(llvm-mos-sdk REQUIRED)

project(14_metatiles2 LANGUAGES C ASM)

set(CMAKE_EXECUTABLE_SUFFIX .nes)

add_executable(metatiles2
  metatiles2.c
  Sprites.c
  chr-rom.s
  BG/Room1.c
)
target_include_directories(metatiles2 PRIVATE .)
target_link_libraries(metatiles2 nesdoug neslib)
set_property(SOURCE chr-rom.s PROPERTY
  OBJECT_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/metatiles2.chr)
