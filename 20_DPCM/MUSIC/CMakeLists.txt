add_library(music music.s)
target_include_directories(music PRIVATE .)
set_property(SOURCE music.s
  PROPERTY OBJECT_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/DMCmusic.dmc)
