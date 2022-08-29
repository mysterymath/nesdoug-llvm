cmake_minimum_required(VERSION 3.18)
set(LLVM_MOS_PLATFORM nes-nrom)
find_package(llvm-mos-sdk REQUIRED)
project(nesdoug LANGUAGES C ASM)

add_subdirectory(01_Hello)
