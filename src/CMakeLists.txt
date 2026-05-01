cmake_minimum_required(VERSION 3.1)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Use vcpkg automatically when VCPKG_ROOT is available.
if(DEFINED ENV{VCPKG_ROOT} AND NOT DEFINED CMAKE_TOOLCHAIN_FILE)
  set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake"
      CACHE STRING "")
endif()
if(DEFINED ENV{VCPKG_DEFAULT_TRIPLET} AND NOT DEFINED VCPKG_TARGET_TRIPLET)
  set(VCPKG_TARGET_TRIPLET "$ENV{VCPKG_DEFAULT_TRIPLET}" CACHE STRING "")
endif()

project(OpenGLBilliards LANGUAGES CXX)

aux_source_directory("./" PROJECT_SOURCES)
add_executable(OpenGLBilliards ${PROJECT_SOURCES})
target_include_directories(OpenGLBilliards PRIVATE include)

set_target_properties(OpenGLBilliards PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}"
)

if(WIN32)
    add_custom_command(TARGET OpenGLBilliards POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "$ENV{VCPKG_ROOT}/installed/x64-windows/bin/glfw3.dll"
        "${CMAKE_SOURCE_DIR}/glfw3.dll"
        COMMENT "Copying glfw3.dll to output directory"
    )
endif()

if(APPLE)
    find_package(glfw3 CONFIG REQUIRED)
    find_package(glm CONFIG REQUIRED)

    target_link_libraries(OpenGLBilliards PRIVATE glfw)
    target_link_libraries(OpenGLBilliards PRIVATE glm::glm)
else()
    find_package(glad CONFIG REQUIRED)
    find_package(glfw3 CONFIG REQUIRED)
    find_package(glm CONFIG REQUIRED)

    target_link_libraries(OpenGLBilliards PRIVATE glad::glad)
    target_link_libraries(OpenGLBilliards PRIVATE glfw)
    target_link_libraries(OpenGLBilliards PRIVATE glm::glm)
endif()
