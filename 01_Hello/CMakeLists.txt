cmake_minimum_required(VERSION 3.18)

set(LLVM_MOS_PLATFORM nes-nrom)
find_package(llvm-mos-sdk REQUIRED)

project(nesdoug LANGUAGES C ASM)

set(CMAKE_EXECUTABLE_SUFFIX .nes)

add_executable(hello hello.c chr-rom.s)
target_include_directories(hello PRIVATE .)
target_link_libraries(hello neslib)
