add_executable(hello hello.c chr-rom.s)
target_include_directories(hello PRIVATE .)
target_link_libraries(hello neslib)
