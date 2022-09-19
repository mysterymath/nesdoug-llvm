cmake_minimum_required(VERSION 3.18)

set(LLVM_MOS_PLATFORM nes-nrom)
find_package(llvm-mos-sdk REQUIRED)

project(16_scroll_up LANGUAGES C ASM)

set(CMAKE_EXECUTABLE_SUFFIX .nes)

add_executable(scroll_up
  scroll_up.c
  Sprites.c
  chr-rom.s
)
target_include_directories(scroll_up PRIVATE .)
target_link_libraries(scroll_up nesdoug neslib rooms)
set_property(SOURCE chr-rom.s PROPERTY
  OBJECT_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/scroll_up.chr)

add_subdirectory(BG)
