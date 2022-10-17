cmake_minimum_required(VERSION 3.18)

set(LLVM_MOS_PLATFORM nes-mmc3)
find_package(llvm-mos-sdk REQUIRED)

project(33_mmc3 LANGUAGES C ASM)

set(CMAKE_EXECUTABLE_SUFFIX .nes)

add_executable(hello_mmc3
  hello_mmc3.c
  chr-rom.s
)
target_include_directories(hello_mmc3 PRIVATE .)
target_link_libraries(hello_mmc3 nesdoug neslib famitone2 music)
target_link_directories(hello_mmc3 PRIVATE .)
set_property(SOURCE chr-rom.s PROPERTY OBJECT_DEPENDS
  ${CMAKE_CURRENT_SOURCE_DIR}/Alpha.chr
  ${CMAKE_CURRENT_SOURCE_DIR}/Gears.chr
)

add_subdirectory(MUSIC)
