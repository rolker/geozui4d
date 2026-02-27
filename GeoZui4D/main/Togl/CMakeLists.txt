add_definitions(-DUSE_TCL_STUBS -DUSE_TK_STUBS)

add_library(gz_togl STATIC togl.c)
set_target_properties(gz_togl PROPERTIES COMPILE_FLAGS -fPIC)

