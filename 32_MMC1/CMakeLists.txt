cmake_minimum_required(VERSION 3.18)

set(LLVM_MOS_PLATFORM nes-mmc1)
find_package(llvm-mos-sdk REQUIRED)

project(32_mmc1 LANGUAGES C ASM)

set(CMAKE_EXECUTABLE_SUFFIX .nes)

add_executable(hello_mmc1
  hello_mmc1.c
  chr-rom.s
)
target_include_directories(hello_mmc1 PRIVATE .)
target_link_libraries(hello_mmc1 nesdoug neslib famitone2 music)
target_link_directories(hello_mmc1 PRIVATE .)
target_link_options(hello_mmc1 PRIVATE -Thello_mmc1.ld)
set_property(SOURCE chr-rom.s PROPERTY OBJECT_DEPENDS
  ${CMAKE_CURRENT_SOURCE_DIR}/Alpha.chr
  ${CMAKE_CURRENT_SOURCE_DIR}/Alpha2.chr
)

add_subdirectory(MUSIC)
