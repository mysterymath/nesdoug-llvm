cmake_minimum_required(VERSION 3.18)

set(LLVM_MOS_PLATFORM nes-nrom)
find_package(llvm-mos-sdk REQUIRED)

project(10_scroll_h LANGUAGES C ASM)

set(CMAKE_EXECUTABLE_SUFFIX .nes)

add_executable(scroll_h
  scroll_h.c
  Sprites.c
  chr-rom.s
  BG/N0.c
  BG/N1.c
  BG/N2.c
)
target_include_directories(scroll_h PRIVATE .)
target_link_libraries(scroll_h nesdoug neslib)
set_property(SOURCE chr-rom.s PROPERTY
  OBJECT_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/big_numbers.chr)
