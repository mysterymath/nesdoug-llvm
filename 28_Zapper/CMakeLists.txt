cmake_minimum_required(VERSION 3.18)

set(LLVM_MOS_PLATFORM nes-nrom)
find_package(llvm-mos-sdk REQUIRED)

project(28_zapper LANGUAGES C ASM)

set(CMAKE_EXECUTABLE_SUFFIX .nes)

add_executable(Zapper
  Zapper.c
  Sprites.c
  chr-rom.s
)
target_include_directories(Zapper PRIVATE .)
target_link_libraries(Zapper nesdoug neslib famitone2 music nes_st)
set_property(SOURCE chr-rom.s PROPERTY
  OBJECT_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/AlphaZ.chr)

add_subdirectory(MUSIC)
add_subdirectory(NES_ST)
