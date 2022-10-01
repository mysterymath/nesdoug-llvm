cmake_minimum_required(VERSION 3.18)

set(LLVM_MOS_PLATFORM nes-nrom)
find_package(llvm-mos-sdk REQUIRED)

project(29_powerpad LANGUAGES C ASM)

set(CMAKE_EXECUTABLE_SUFFIX .nes)

add_executable(PowerTest
  PowerTest.c
  chr-rom.s
)
target_include_directories(PowerTest PRIVATE .)
target_link_libraries(PowerTest nesdoug neslib nes_st)
set_property(SOURCE chr-rom.s PROPERTY
  OBJECT_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/AlphaP.chr)

add_subdirectory(NES_ST)
