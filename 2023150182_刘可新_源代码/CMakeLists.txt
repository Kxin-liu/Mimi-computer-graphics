# cmake version
cmake_minimum_required(VERSION 3.1)

# c++11
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# vcpkg
if(DEFINED ENV{VCPKG_ROOT} AND NOT DEFINED CMAKE_TOOLCHAIN_FILE)
  set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake"
      CACHE STRING "")
endif()
if(DEFINED ENV{VCPKG_DEFAULT_TRIPLET} AND NOT DEFINED VCPKG_TARGET_TRIPLET)
  set(VCPKG_TARGET_TRIPLET "$ENV{VCPKG_DEFAULT_TRIPLET}" CACHE STRING "")
endif()

# project
project(main)

# target
aux_source_directory("./" PROJECT_SOURCES)
add_executable(main ${PROJECT_SOURCES})
target_include_directories(main PRIVATE include)

# 设置可执行文件的输出路径
set_target_properties(main PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}"  # 输出到项目根目录
)

# 复制DLL到输出目录
if(WIN32)
    add_custom_command(TARGET main POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "$ENV{VCPKG_ROOT}/installed/x64-windows/bin/glfw3.dll"
        "${CMAKE_SOURCE_DIR}/glfw3.dll"
        COMMENT "Copying glfw3.dll to output directory"
    )
endif()

if(APPLE)

	find_package(glfw3 CONFIG REQUIRED)
	find_package(glm CONFIG REQUIRED)
	# link

   	target_link_libraries(main PRIVATE glfw)
	target_link_libraries(main PRIVATE glm::glm)
else()
	# dependency
	find_package(glad CONFIG REQUIRED)
	find_package(glfw3 CONFIG REQUIRED)
	find_package(glm CONFIG REQUIRED)
	# link
	target_link_libraries(main PRIVATE glad::glad)
   	target_link_libraries(main PRIVATE glfw)
	target_link_libraries(main PRIVATE glm::glm)
endif(APPLE)
