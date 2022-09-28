cmake_minimum_required(VERSION 3.18)

set(LLVM_MOS_PLATFORM nes-cnrom)
find_package(llvm-mos-sdk REQUIRED)

project(24_mappers LANGUAGES C ASM)

set(CMAKE_EXECUTABLE_SUFFIX .nes)

add_executable(mappers
  mappers.c
  chr-rom.s
  NES_ST/all_bgs.c
)
target_include_directories(mappers PRIVATE .)
target_link_libraries(mappers nesdoug neslib)

set_property(SOURCE chr-rom.s PROPERTY OBJECT_DEPENDS
  ${CMAKE_CURRENT_SOURCE_DIR}/apples.chr
  ${CMAKE_CURRENT_SOURCE_DIR}/balls.chr
  ${CMAKE_CURRENT_SOURCE_DIR}/flower.chr
  ${CMAKE_CURRENT_SOURCE_DIR}/snake.chr
)
