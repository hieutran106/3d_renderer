cmake_minimum_required(VERSION 3.24) # 3.24+ is best for FetchContent
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
    if (APPLE)
        # Remove Quarantine Attribute
        execute_process(COMMAND xattr -r -d com.apple.quarantine "${SDL3_FRAMEWORK}" OUTPUT_QUIET ERROR_QUIET)
    endif ()
    message(STATUS ${SDL3_FRAMEWORK})
    target_link_libraries(${target_name} PRIVATE "${SDL3_FRAMEWORK}")
    set_target_properties(${target_name} PROPERTIES
            XCODE_EMBED_FRAMEWORKS "${SDL3_FRAMEWORK}"
            XCODE_EMBED_FRAMEWORKS_CODE_SIGN_ON_COPY ON
    )
endfunction()

function(link_sdl3_ttf_xc target_name)
    find_library(SDL3_TTF_FRAMEWORK
            NAMES SDL3_ttf
            PATHS "${CMAKE_SOURCE_DIR}/third_party/"
            REQUIRED
    )
    if (NOT SDL3_TTF_FRAMEWORK)
        message(FATAL_ERROR "SDL3_ttf.xcframework not found!")
    endif ()
    if (APPLE)
        # Remove Quarantine Attribute
        execute_process(COMMAND xattr -r -d com.apple.quarantine "${SDL3_TTF_FRAMEWORK}" OUTPUT_QUIET ERROR_QUIET)
    endif ()
    message(STATUS ${SDL3_TTF_FRAMEWORK})
    target_link_libraries(${target_name} PRIVATE "${SDL3_TTF_FRAMEWORK}")
    set_target_properties(${target_name} PROPERTIES
            XCODE_EMBED_FRAMEWORKS "${SDL3_TTF_FRAMEWORK}"
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
link_sdl3_ttf_xc(3d_renderer)

option(USE_SANITIZERS "Enable ASan and UBSan" ON)
if (USE_SANITIZERS)
    message(STATUS "Building with Address and Undefined Behavior Sanitizers")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address,undefined -fno-omit-frame-pointer")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=address,undefined")
endif ()