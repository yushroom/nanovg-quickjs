cmake_minimum_required(VERSION 3.0.0)

project(nanovg-quickjs)

set(QUICKJS_ROOT_DIR "./thirdparty/QuickJS")
add_library(libquickjs
    ${QUICKJS_ROOT_DIR}/cutils.c
    ${QUICKJS_ROOT_DIR}/cutils.h
    ${QUICKJS_ROOT_DIR}/libbf.c
    ${QUICKJS_ROOT_DIR}/libbf.h
    ${QUICKJS_ROOT_DIR}/libregexp.c
    ${QUICKJS_ROOT_DIR}/libregexp.h
    ${QUICKJS_ROOT_DIR}/libunicode.c
    ${QUICKJS_ROOT_DIR}/libunicode.h
    ${QUICKJS_ROOT_DIR}/list.h
    ${QUICKJS_ROOT_DIR}/quickjs-atom.h
    ${QUICKJS_ROOT_DIR}/quickjs-libc.c
    ${QUICKJS_ROOT_DIR}/quickjs-libc.h
    ${QUICKJS_ROOT_DIR}/quickjs-opcode.h
    ${QUICKJS_ROOT_DIR}/quickjs.c
    ${QUICKJS_ROOT_DIR}/quickjs.h
)

target_compile_definitions(libquickjs PUBLIC CONFIG_VERSION="2020-01-19")
target_compile_definitions(libquickjs PUBLIC _GNU_SOURCE)
target_compile_definitions(libquickjs PUBLIC CONFIG_BIGNUM)
target_include_directories(libquickjs PUBLIC ${QUICKJS_ROOT_DIR})

add_subdirectory(./thirdparty/glfw)

set(NANOVG_ROOT_DIR "./thirdparty/nanovg")
add_library(nanovg
    ${NANOVG_ROOT_DIR}/src/nanovg.c
    ${NANOVG_ROOT_DIR}/src/nanovg.h
)
target_include_directories(nanovg PUBLIC ${NANOVG_ROOT_DIR}/src)

set(SRC_ROOT_DIR "./src")
add_library(nanovg_qjs
    ${SRC_ROOT_DIR}/nanovg_qjs.c
    ${SRC_ROOT_DIR}/nanovg_qjs.h
)
target_link_libraries(nanovg_qjs PUBLIC nanovg libquickjs)
target_include_directories(nanovg_qjs PUBLIC ${SRC_ROOT_DIR})

add_executable(nanovg_example
    ${NANOVG_ROOT_DIR}/example/demo.c
    ${NANOVG_ROOT_DIR}/example/demo.h
    ${NANOVG_ROOT_DIR}/example/perf.h
    ${NANOVG_ROOT_DIR}/example/perf.c
    ${NANOVG_ROOT_DIR}/example/example_gl3.c
)
target_include_directories(nanovg_example PUBLIC ${NANOVG_ROOT_DIR}/example)
target_link_libraries(nanovg_example PUBLIC nanovg glfw)
target_link_libraries(nanovg_example PUBLIC nanovg_qjs)
set_property(TARGET nanovg_example APPEND_STRING PROPERTY LINK_FLAGS "-framework OpenGL -framework Cocoa -framework IOKit -framework CoreVideo -framework Carbon")
target_compile_definitions(nanovg_example PUBLIC GL_SILENCE_DEPRECATION)


add_library(nanovg_metal
    ./thirdparty/MetalNanoVG/src/nanovg_mtl.h
    ./thirdparty/MetalNanoVG/src/nanovg_mtl.m
    ./thirdparty/MetalNanoVG/src/nanovg_mtl_shaders.metal
)
target_link_libraries(nanovg_metal PUBLIC nanovg)
target_include_directories(nanovg_metal PUBLIC ./thirdparty/MetalNanoVG/src)
set_property(TARGET nanovg_metal APPEND_STRING PROPERTY COMPILE_FLAGS "-fobjc-arc")

add_executable(nanovg_example_qjs
    ${NANOVG_ROOT_DIR}/example/demo.c
    ${NANOVG_ROOT_DIR}/example/demo.h
    ${NANOVG_ROOT_DIR}/example/perf.h
    ${NANOVG_ROOT_DIR}/example/perf.c
    ./src/example_gl3.c
    ./src/nvg.js
)
target_include_directories(nanovg_example_qjs PUBLIC ${NANOVG_ROOT_DIR}/example)
target_link_libraries(nanovg_example_qjs PUBLIC nanovg glfw)
target_link_libraries(nanovg_example_qjs PUBLIC nanovg_qjs)
set_property(TARGET nanovg_example_qjs APPEND_STRING PROPERTY COMPILE_FLAGS "-fobjc-arc")
set_property(TARGET nanovg_example_qjs APPEND_STRING PROPERTY LINK_FLAGS "-framework OpenGL -framework Cocoa -framework IOKit -framework CoreVideo -framework Carbon")
target_compile_definitions(nanovg_example_qjs PUBLIC GL_SILENCE_DEPRECATION)
# set_property(TARGET nanovg_example_metal APPEND_STRING PROPERTY LINK_FLAGS "-framework Cocoa -framework MetalKit -framework Metal -framework QuartzCore")
