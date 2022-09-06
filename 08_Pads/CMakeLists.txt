cmake_minimum_required(VERSION 3.18)

set(LLVM_MOS_PLATFORM nes-nrom)
find_package(llvm-mos-sdk REQUIRED)

project(08_pads LANGUAGES C ASM)

set(CMAKE_EXECUTABLE_SUFFIX .nes)

add_executable(Pads Pads.c Sprites.c chr-rom.s)
target_include_directories(Pads PRIVATE .)
target_link_libraries(Pads nesdoug neslib)
set_property(SOURCE chr-rom.s PROPERTY
  OBJECT_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/Alpha3.chr)
