cmake_minimum_required(VERSION 3.18)

set(LLVM_MOS_PLATFORM nes-nrom)
find_package(llvm-mos-sdk REQUIRED)

project(08_bg_collide LANGUAGES C ASM)

set(CMAKE_EXECUTABLE_SUFFIX .nes)

add_executable(collide
  collide.c
  Sprites.c
  chr-rom.s
  CSV/c1.c
  CSV/c2.c
  CSV/c3.c
  CSV/c4.c
)
target_include_directories(collide PRIVATE .)
target_link_libraries(collide nesdoug neslib)
set_property(SOURCE chr-rom.s PROPERTY
  OBJECT_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/collide.chr)
