add_executable(hello hello.c chr-rom.s)
set_property(TARGET hello PROPERTY OUTPUT_NAME hello.nes)
target_include_directories(hello PRIVATE .)
target_link_libraries(hello neslib)
