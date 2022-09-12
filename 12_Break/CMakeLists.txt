cmake_minimum_required(VERSION 3.18)

set(LLVM_MOS_PLATFORM nes-nrom)
find_package(llvm-mos-sdk REQUIRED)

project(12_breaky LANGUAGES C ASM)

set(CMAKE_EXECUTABLE_SUFFIX .nes)

add_executable(breaky
  breaky.c
  Sprites.c
  chr-rom.s
  BG/breaky_bg2.c
  CSV/c1.c
)
target_include_directories(breaky PRIVATE .)
target_link_libraries(breaky nesdoug neslib)
set_property(SOURCE chr-rom.s PROPERTY
  OBJECT_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/breaky.chr)
