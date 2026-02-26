cmake_minimum_required(VERSION 4.0) # 3.24+ is best for FetchContent
project(3d_renderer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

function(link_sdl3_xc target_name)
    find_library(SDL3_FRAMEWORK
            NAMES SDL3
            PATHS "${CMAKE_SOURCE_DIR}/third_party/"
            REQUIRED
    )
    if (NOT SDL3_FRAMEWORK)
        message(FATAL_ERROR "SDL3.xcframework not found!")
    endif ()
    message(STATUS ${SDL3_FRAMEWORK})
    target_link_libraries(${target_name} PRIVATE "${SDL3_FRAMEWORK}")
    set_target_properties(${target_name} PROPERTIES
            XCODE_EMBED_FRAMEWORKS "${SDL3_FRAMEWORK}"
            XCODE_EMBED_FRAMEWORKS_CODE_SIGN_ON_COPY ON
    )
endfunction()

function(get_cpp_sources target_name result)
    file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/src/*.cpp")
    set(${result} ${SOURCES} PARENT_SCOPE)
    # debug
    message(STATUS "get_cpp_sources:")
    foreach (src IN LISTS SOURCES)
        message(STATUS "    ${src}")
    endforeach ()
endfunction()

get_cpp_sources(3d_renderer SOURCES)
add_executable(3d_renderer ${SOURCES})
link_sdl3_xc(3d_renderer)
target_compile_options(3d_renderer PRIVATE -fsanitize=address -fno-omit-frame-pointer -g)
target_link_options(3d_renderer PRIVATE -fsanitize=address)