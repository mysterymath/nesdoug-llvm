cmake_minimum_required(VERSION 3.18)

set(LLVM_MOS_PLATFORM nes-nrom)
find_package(llvm-mos-sdk REQUIRED)

project(15_scroll_r LANGUAGES C ASM)

set(CMAKE_EXECUTABLE_SUFFIX .nes)

add_executable(scroll_r
  scroll_r.c
  Sprites.c
  chr-rom.s
)
target_include_directories(scroll_r PRIVATE .)
target_link_libraries(scroll_r nesdoug neslib rooms)
set_property(SOURCE chr-rom.s PROPERTY
  OBJECT_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/scroll_r.chr)

add_subdirectory(BG)
