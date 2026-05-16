cmake_minimum_required(VERSION 3.24) # 3.24+ is best for FetchContent
project(3d_renderer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# Enable the compilation database export
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

function(link_sdl3_xc target_name)
    #    find_library(SDL3_FRAMEWORK
    #            NAMES SDL3
    #            PATHS "${CMAKE_SOURCE_DIR}/third_party/"
    #            REQUIRED
    #    )
    set(SDL3_FRAMEWORK "${CMAKE_SOURCE_DIR}/third_party/SDL3.xcframework")
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

function(link_imgui target_name)
    set(IMGUI_DIR "${CMAKE_CURRENT_SOURCE_DIR}/third_party/imgui")
    set(IMGUI_SOURCES
            ${IMGUI_DIR}/imgui.cpp
            ${IMGUI_DIR}/imgui_demo.cpp
            ${IMGUI_DIR}/imgui_draw.cpp
            ${IMGUI_DIR}/imgui_tables.cpp
            ${IMGUI_DIR}/imgui_widgets.cpp
            # Backends
            ${IMGUI_DIR}/backends/imgui_impl_sdl3.cpp
            ${IMGUI_DIR}/backends/imgui_impl_sdlrenderer3.cpp
    )


    message(STATUS "link_imgui:")
    foreach (src IN LISTS IMGUI_SOURCES)
        message(STATUS "    ${src}")
    endforeach ()

    target_include_directories(${target_name} PRIVATE
            ${IMGUI_DIR}
            ${IMGUI_DIR}/backends
    )
    target_sources(${target_name} PRIVATE ${IMGUI_SOURCES})
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

function(set_target_props target_name)
    if (IOS)
        set_target_properties(${target_name} PROPERTIES
                MACOSX_BUNDLE TRUE
                MACOSX_BUNDLE_BUNDLE_NAME "3d_renderer"
                MACOSX_BUNDLE_GUI_IDENTIFIER "Boeing.bgs.vast.poc.3drenderer"
                MACOSX_BUNDLE_BUNDLE_VERSION "1.0.0"
                MACOSX_BUNDLE_SHORT_VERSION_STRING "1.0"
                MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/ios/Info.plist.in

                XCODE_ATTRIBUTE_PRODUCT_NAME "3d_renderer"
                XCODE_ATTRIBUTE_ASSETCATALOG_COMPILER_APPICON_NAME "AppIcon"
                XCODE_ATTRIBUTE_PRODUCT_BUNDLE_IDENTIFIER "Boeing.bgs.vast.poc.3drenderer"
                XCODE_ATTRIBUTE_IPHONEOS_DEPLOYMENT_TARGET "15.0"
                XCODE_ATTRIBUTE_TARGETED_DEVICE_FAMILY "1,2"
                BUILD_WITH_INSTALL_RPATH TRUE
                INSTALL_RPATH "@executable_path/Frameworks"
                XCODE_ATTRIBUTE_LD_RUNPATH_SEARCH_PATHS "@executable_path/Frameworks"
        )

    endif ()
endfunction()

function(copy_asset target_name)
    if (IOS)
        add_custom_command(TARGET ${target_name} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_directory
                "${CMAKE_SOURCE_DIR}/assets"
                "$<TARGET_BUNDLE_DIR:${target_name}>/assets"
                COMMENT "Copying assets folder into iOS app bundle"
        )
        message(STATUS "Copy asset")
    elseif (APPLE AND NOT IOS)
        # For macOS, this copies assets next to the binary
        add_custom_command(TARGET ${target_name} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E echo "DEBUG: Running post-build copy..."
                COMMAND ${CMAKE_COMMAND} -E copy_directory
                "${CMAKE_SOURCE_DIR}/assets"
                "$<TARGET_FILE_DIR:${target_name}>/assets"
                COMMENT "Copying assets folder next to macOS executable"
        )
    endif ()

endfunction()

get_cpp_sources(3d_renderer SOURCES)
add_executable(3d_renderer ${SOURCES})

link_sdl3_xc(3d_renderer)
link_imgui(3d_renderer)

set_target_props(3d_renderer)

option(USE_SANITIZERS "Enable ASan and UBSan" OFF)
if (USE_SANITIZERS)
    message(STATUS "Building with Address and Undefined Behavior Sanitizers")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address,undefined -fno-omit-frame-pointer")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=address,undefined")
endif ()

copy_asset(3d_renderer)