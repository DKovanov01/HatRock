
cmake_minimum_required(VERSION 3.25)

project(VulkanGameEngine
    VERSION 0.1.0
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# --------------------------------------------------
# Dependency roots
# --------------------------------------------------

set(ENGINE_DEPS_ROOT "C:/EngineDev")

set(SDL3_ROOT "${ENGINE_DEPS_ROOT}/SDL3")
set(GLEW_ROOT "${ENGINE_DEPS_ROOT}/OpenGL")
set(MATH_ROOT "${ENGINE_DEPS_ROOT}/MathLibrary")
set(ENTT_ROOT "${ENGINE_DEPS_ROOT}/EnTT")
set(PHYSX_ROOT "${ENGINE_DEPS_ROOT}/PhysX/physx")
set(PHYSX_BIN "${PHYSX_ROOT}/bin/win.x86_64.vc143.md")
set(PHYSX_DEBUG_BIN "${PHYSX_BIN}/debug")
set(PHYSX_RELEASE_BIN "${PHYSX_BIN}/release")

# --------------------------------------------------
# SDL3
# Expected:
# SDL3/include/SDL3/SDL.h
# SDL3/lib/x64/SDL3.lib
# SDL3/dll/x64/SDL3.dll
# --------------------------------------------------

if(NOT EXISTS "${SDL3_ROOT}/include/SDL3/SDL.h")
    message(FATAL_ERROR
        "SDL3 header not found: ${SDL3_ROOT}/include/SDL3/SDL.h")
endif()

find_library(SDL3_LIBRARY
    NAMES SDL3
    PATHS
        "${SDL3_ROOT}/lib/x64"
    NO_DEFAULT_PATH
)

find_file(SDL3_DLL
    NAMES SDL3.dll
    PATHS
        "${SDL3_ROOT}/dll/x64"
    NO_DEFAULT_PATH
)

if(NOT SDL3_LIBRARY)
    message(FATAL_ERROR
        "SDL3.lib not found in ${SDL3_ROOT}/lib/x64")
endif()

if(NOT SDL3_DLL)
    message(FATAL_ERROR
        "SDL3.dll not found in ${SDL3_ROOT}/dll/x64")
endif()

add_library(SDL3::SDL3 SHARED IMPORTED GLOBAL)

set_target_properties(SDL3::SDL3 PROPERTIES
    IMPORTED_IMPLIB "${SDL3_LIBRARY}"
    IMPORTED_LOCATION "${SDL3_DLL}"
    INTERFACE_INCLUDE_DIRECTORIES "${SDL3_ROOT}/include"
)

# --------------------------------------------------
# SDL3_image
# --------------------------------------------------

if(NOT EXISTS "${SDL3_ROOT}/include/SDL3_image/SDL_image.h")
    message(FATAL_ERROR
        "SDL3_image header not found.")
endif()

find_library(SDL3_IMAGE_LIBRARY
    NAMES SDL3_image
    PATHS "${SDL3_ROOT}/lib/x64"
    NO_DEFAULT_PATH
)

find_file(SDL3_IMAGE_DLL
    NAMES SDL3_image.dll
    PATHS "${SDL3_ROOT}/dll/x64"
    NO_DEFAULT_PATH
)

if(NOT SDL3_IMAGE_LIBRARY OR NOT SDL3_IMAGE_DLL)
    message(FATAL_ERROR
        "SDL3_image library or DLL not found. Check SDL3/lib/x64 and SDL3/dll/x64.")
endif()

add_library(SDL3_image::SDL3_image SHARED IMPORTED GLOBAL)

set_target_properties(SDL3_image::SDL3_image PROPERTIES
    IMPORTED_IMPLIB "${SDL3_IMAGE_LIBRARY}"
    IMPORTED_LOCATION "${SDL3_IMAGE_DLL}"
    INTERFACE_INCLUDE_DIRECTORIES "${SDL3_ROOT}/include"
    INTERFACE_LINK_LIBRARIES SDL3::SDL3
)

# --------------------------------------------------
# SDL3_net
# --------------------------------------------------

if(NOT EXISTS "${SDL3_ROOT}/include/SDL3_net/SDL_net.h")
    message(FATAL_ERROR
        "SDL3_net header not found.")
endif()

find_library(SDL3_NET_LIBRARY
    NAMES SDL3_net
    PATHS "${SDL3_ROOT}/lib/x64"
    NO_DEFAULT_PATH
)

find_file(SDL3_NET_DLL
    NAMES SDL3_net.dll
    PATHS "${SDL3_ROOT}/dll/x64"
    NO_DEFAULT_PATH
)

if(NOT SDL3_NET_LIBRARY OR NOT SDL3_NET_DLL)
    message(FATAL_ERROR
        "SDL3_net library or DLL not found. Check SDL3/lib/x64 and SDL3/dll/x64.")
endif()

add_library(SDL3_net::SDL3_net SHARED IMPORTED GLOBAL)

set_target_properties(SDL3_net::SDL3_net PROPERTIES
    IMPORTED_IMPLIB "${SDL3_NET_LIBRARY}"
    IMPORTED_LOCATION "${SDL3_NET_DLL}"
    INTERFACE_INCLUDE_DIRECTORIES "${SDL3_ROOT}/include"
    INTERFACE_LINK_LIBRARIES SDL3::SDL3
)

# --------------------------------------------------
# Vulkan SDK
# Uses the installed Vulkan SDK.
# --------------------------------------------------

find_package(Vulkan REQUIRED)

# --------------------------------------------------
# OpenGL and GLEW
# --------------------------------------------------

find_package(OpenGL REQUIRED)

if(NOT EXISTS "${GLEW_ROOT}/include/glew.h")
    message(FATAL_ERROR
        "GLEW header not found: ${GLEW_ROOT}/include/glew.h")
endif()

# Prefer the x64 library directory.
# Fall back to the library root if necessary.
find_library(GLEW_LIBRARY
    NAMES glew32
    PATHS
        "${GLEW_ROOT}/lib/x64"
        "${GLEW_ROOT}/lib"
    NO_DEFAULT_PATH
)

find_file(GLEW_DLL
    NAMES glew32.dll
    PATHS
        "${GLEW_ROOT}/dll/x64"
        "${GLEW_ROOT}/dll"
    NO_DEFAULT_PATH
)

if(NOT GLEW_LIBRARY)
    message(FATAL_ERROR
        "glew32.lib not found in the OpenGL library folders.")
endif()

if(NOT GLEW_DLL)
    message(FATAL_ERROR
        "glew32.dll not found in the OpenGL DLL folders.")
endif()

add_library(GLEW::GLEW SHARED IMPORTED GLOBAL)

set_target_properties(GLEW::GLEW PROPERTIES
    IMPORTED_IMPLIB "${GLEW_LIBRARY}"
    IMPORTED_LOCATION "${GLEW_DLL}"
    INTERFACE_INCLUDE_DIRECTORIES "${GLEW_ROOT}/include"
)

# --------------------------------------------------
# MathLibrary
# --------------------------------------------------

file(GLOB_RECURSE MATH_LIBRARY_SOURCES
    CONFIGURE_DEPENDS
    "${MATH_ROOT}/Include/*.cpp"
)

if(MATH_LIBRARY_SOURCES)
    add_library(MathLibrary STATIC
        ${MATH_LIBRARY_SOURCES}
    )

    target_include_directories(MathLibrary
        PUBLIC
            "${MATH_ROOT}/Include"
            "${GLEW_ROOT}/include"
    )
else()
    add_library(MathLibrary INTERFACE)

    target_include_directories(MathLibrary
        INTERFACE
            "${MATH_ROOT}/Include"
            "${GLEW_ROOT}/include"
    )
endif()

# --------------------------------------------------
# EnTT
# --------------------------------------------------

if(NOT EXISTS "${ENTT_ROOT}/src/entt/entt.hpp")
    message(FATAL_ERROR "EnTT headers not found.")
endif()

add_library(EnTT INTERFACE)
add_library(EnTT::EnTT ALIAS EnTT)

target_include_directories(EnTT INTERFACE
    "${ENTT_ROOT}/src"
)

# --------------------------------------------------
# PhysX
# --------------------------------------------------

if(NOT EXISTS "${PHYSX_ROOT}/include/PxPhysicsAPI.h")
    message(FATAL_ERROR "PhysX headers not found.")
endif()

foreach(PHYSX_CONFIG DEBUG RELEASE)
    if(PHYSX_CONFIG STREQUAL "DEBUG")
        set(PHYSX_CONFIG_BIN "${PHYSX_DEBUG_BIN}")
    else()
        set(PHYSX_CONFIG_BIN "${PHYSX_RELEASE_BIN}")
    endif()

# Verify PhysX runtime DLLs
foreach(PHYSX_DLL
    PhysX_64.dll
    PhysXCommon_64.dll
    PhysXFoundation_64.dll
)
    if(NOT EXISTS "${PHYSX_CONFIG_BIN}/${PHYSX_DLL}")
            message(FATAL_ERROR
                "Missing PhysX ${PHYSX_CONFIG} DLL: ${PHYSX_CONFIG_BIN}/${PHYSX_DLL}")
        endif()
    endforeach()

# Verify PhysX import and static libraries
foreach(PHYSX_LIB
    PhysX_64.lib
    PhysXCommon_64.lib
    PhysXFoundation_64.lib
    PhysXExtensions_static_64.lib
    PhysXPvdSDK_static_64.lib
)
    if(NOT EXISTS "${PHYSX_CONFIG_BIN}/${PHYSX_LIB}")
            message(FATAL_ERROR
                "Missing PhysX ${PHYSX_CONFIG} library: ${PHYSX_CONFIG_BIN}/${PHYSX_LIB}")
        endif()
    endforeach()
endforeach()

# PhysX Foundation
add_library(PhysX::Foundation SHARED IMPORTED GLOBAL)
set_target_properties(PhysX::Foundation PROPERTIES
    IMPORTED_CONFIGURATIONS "DEBUG;RELEASE"

    IMPORTED_IMPLIB_DEBUG
        "${PHYSX_DEBUG_BIN}/PhysXFoundation_64.lib"
    IMPORTED_LOCATION_DEBUG
        "${PHYSX_DEBUG_BIN}/PhysXFoundation_64.dll"

    IMPORTED_IMPLIB_RELEASE
        "${PHYSX_RELEASE_BIN}/PhysXFoundation_64.lib"
    IMPORTED_LOCATION_RELEASE
        "${PHYSX_RELEASE_BIN}/PhysXFoundation_64.dll"
)

# PhysX Common
add_library(PhysX::Common SHARED IMPORTED GLOBAL)
set_target_properties(PhysX::Common PROPERTIES
    IMPORTED_CONFIGURATIONS "DEBUG;RELEASE"

    IMPORTED_IMPLIB_DEBUG
        "${PHYSX_DEBUG_BIN}/PhysXCommon_64.lib"
    IMPORTED_LOCATION_DEBUG
        "${PHYSX_DEBUG_BIN}/PhysXCommon_64.dll"

    IMPORTED_IMPLIB_RELEASE
        "${PHYSX_RELEASE_BIN}/PhysXCommon_64.lib"
    IMPORTED_LOCATION_RELEASE
        "${PHYSX_RELEASE_BIN}/PhysXCommon_64.dll"
)

# PhysX Core
add_library(PhysX::Core SHARED IMPORTED GLOBAL)
set_target_properties(PhysX::Core PROPERTIES
    IMPORTED_CONFIGURATIONS "DEBUG;RELEASE"

    IMPORTED_IMPLIB_DEBUG
        "${PHYSX_DEBUG_BIN}/PhysX_64.lib"
    IMPORTED_LOCATION_DEBUG
        "${PHYSX_DEBUG_BIN}/PhysX_64.dll"

    IMPORTED_IMPLIB_RELEASE
        "${PHYSX_RELEASE_BIN}/PhysX_64.lib"
    IMPORTED_LOCATION_RELEASE
        "${PHYSX_RELEASE_BIN}/PhysX_64.dll"
)

# PhysX Extensions
add_library(PhysX::Extensions STATIC IMPORTED GLOBAL)
set_target_properties(PhysX::Extensions PROPERTIES
    IMPORTED_CONFIGURATIONS "DEBUG;RELEASE"

    IMPORTED_LOCATION_DEBUG
        "${PHYSX_DEBUG_BIN}/PhysXExtensions_static_64.lib"
    IMPORTED_LOCATION_RELEASE
        "${PHYSX_RELEASE_BIN}/PhysXExtensions_static_64.lib"
)

# PhysX Visual Debugger SDK
add_library(PhysX::PvdSDK STATIC IMPORTED GLOBAL)
set_target_properties(PhysX::PvdSDK PROPERTIES
    IMPORTED_CONFIGURATIONS "DEBUG;RELEASE"

    IMPORTED_LOCATION_DEBUG
        "${PHYSX_DEBUG_BIN}/PhysXPvdSDK_static_64.lib"
    IMPORTED_LOCATION_RELEASE
        "${PHYSX_RELEASE_BIN}/PhysXPvdSDK_static_64.lib"
)

# --------------------------------------------------
# Collect HatRock engine source files
# --------------------------------------------------

file(GLOB_RECURSE HATROCK_SOURCES
    CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/HatRock/*.cpp"
)

file(GLOB_RECURSE HATROCK_HEADERS
    CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/HatRock/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/HatRock/*.hpp"
)

# Remove main.cpp from the library sources.
set(HATROCK_MAIN
    "${CMAKE_CURRENT_SOURCE_DIR}/HatRock/Main.cpp"
)

list(REMOVE_ITEM HATROCK_SOURCES "${HATROCK_MAIN}")

# --------------------------------------------------
# Engine library
# --------------------------------------------------

add_library(HatRockEngine STATIC
    ${HATROCK_SOURCES}
    ${HATROCK_HEADERS}
)

target_include_directories(HatRockEngine
    PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/HatRock"
        "${MATH_ROOT}/Include"
        "${GLEW_ROOT}/include"
		"${PHYSX_ROOT}/include"
)

target_link_libraries(HatRockEngine
    PUBLIC
        SDL3::SDL3
		SDL3_image::SDL3_image
        SDL3_net::SDL3_net
        Vulkan::Vulkan
        OpenGL::GL
        GLEW::GLEW
        MathLibrary
		EnTT::EnTT
		PhysX::Extensions
		PhysX::PvdSDK
		PhysX::Core
		PhysX::Common
		PhysX::Foundation
)

# --------------------------------------------------
# Organize files in Visual Studio's CMake Solution Explorer
# --------------------------------------------------

# Include every engine file and the application entry point.
set(HATROCK_ALL_FILES
    ${HATROCK_SOURCES}
    ${HATROCK_HEADERS}
    ${HATROCK_MAIN}
)

# Collect files for each virtual folder by matching their full paths.
set(HATROCK_CORE_FILES ${HATROCK_ALL_FILES})
list(FILTER HATROCK_CORE_FILES INCLUDE
    REGEX "/(Main|Timer|Debug|Actor|Camera)\\.(cpp|h|hpp)$"
)
source_group("HatRock\\Core" FILES ${HATROCK_CORE_FILES})

set(HATROCK_VULKAN_FILES ${HATROCK_ALL_FILES})
list(FILTER HATROCK_VULKAN_FILES INCLUDE
    REGEX "/(Vulkan[^/]*|DescriptorSetBuilder)\\.(cpp|h|hpp)$"
)
source_group("HatRock\\Rendering\\Vulkan" FILES ${HATROCK_VULKAN_FILES})

set(HATROCK_OPENGL_FILES ${HATROCK_ALL_FILES})
list(FILTER HATROCK_OPENGL_FILES INCLUDE
    REGEX "/OpenGLRenderer\\.(cpp|h|hpp)$"
)
source_group("HatRock\\Rendering\\OpenGL" FILES ${HATROCK_OPENGL_FILES})

set(HATROCK_SCENE_FILES ${HATROCK_ALL_FILES})
list(FILTER HATROCK_SCENE_FILES INCLUDE
    REGEX "/(Scene0|SceneManager)\\.(cpp|h|hpp)$"
)
source_group("HatRock\\Scene" FILES ${HATROCK_SCENE_FILES})

# --------------------------------------------------
# Actual HatRock application
# --------------------------------------------------

add_executable(HatRock
    "${HATROCK_MAIN}"
)

target_link_libraries(HatRock
    PRIVATE HatRockEngine
)

# Copy runtime DLLs beside the HatRock executable
add_custom_command(TARGET HatRock POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${SDL3_DLL}"
        "$<TARGET_FILE_DIR:HatRock>"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${SDL3_IMAGE_DLL}"
        "$<TARGET_FILE_DIR:HatRock>"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${SDL3_NET_DLL}"
        "$<TARGET_FILE_DIR:HatRock>"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${GLEW_DLL}"
        "$<TARGET_FILE_DIR:HatRock>"

    # PhysX DLLs - automatically select Debug or Release
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "$<IF:$<CONFIG:Debug>,${PHYSX_DEBUG_BIN},${PHYSX_RELEASE_BIN}>/PhysX_64.dll"
        "$<TARGET_FILE_DIR:HatRock>"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "$<IF:$<CONFIG:Debug>,${PHYSX_DEBUG_BIN},${PHYSX_RELEASE_BIN}>/PhysXCommon_64.dll"
        "$<TARGET_FILE_DIR:HatRock>"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "$<IF:$<CONFIG:Debug>,${PHYSX_DEBUG_BIN},${PHYSX_RELEASE_BIN}>/PhysXFoundation_64.dll"
        "$<TARGET_FILE_DIR:HatRock>"
)

# Copy runtime assets beside the executable
add_custom_command(TARGET HatRock POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/Assets"
        "$<TARGET_FILE_DIR:HatRock>/Assets"
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/Shaders"
        "$<TARGET_FILE_DIR:HatRock>/Shaders"
)