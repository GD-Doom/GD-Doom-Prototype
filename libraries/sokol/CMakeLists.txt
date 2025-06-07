
add_library(sokol sokol.cc)

set (SOKOL_LINK_LIBRARIES OpenGL::GL)

target_link_libraries(sokol PUBLIC ${SOKOL_LINK_LIBRARIES})

target_include_directories(sokol PUBLIC ./)
target_compile_definitions(sokol PUBLIC SOKOL_NO_DEPRECATED)

target_compile_definitions (sokol PUBLIC SOKOL_GLCORE)
