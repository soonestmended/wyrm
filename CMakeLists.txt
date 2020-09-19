cmake_minimum_required(VERSION 3.1.0)
project(wyrm)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

#set(CMAKE_CXX_FLAGS "-Wall")

if (MSVC)
	set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE "../x64-release")
	set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG "../x64-debug/")
else ()
	set(CMAKE_CXX_FLAGS_DEBUG "-g")
	set(CMAKE_CXX_FLAGS_RELEASE "-O3")
	set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE "../release/")
	set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG "../debug/")
endif()

message(STATUS "CXX_FLAGS_DEBUG: ${CMAKE_CXX_FLAGS_DEBUG}")
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

include_directories(include)

file(GLOB SOURCES "src/*.cpp")

message("INTERACTIVE_MODE = ${INTERACTIVE_MODE}")

if (INTERACTIVE_MODE)
	message("Compiling with interactive mode...")
	add_compile_definitions(INTERACTIVE_MODE)
	include_directories("glad/include" "glfw/include" "imgui/")
	file(GLOB INTERACTIVE_SOURCES "src/interactive/*.cpp" "imgui/*.cpp")
	list(APPEND SOURCES ${INTERACTIVE_SOURCES})
	list(APPEND SOURCES "glad/src/glad.c")
endif()

add_executable(wyrm ${SOURCES})

if (INTERACTIVE_MODE)
	target_link_libraries(wyrm PRIVATE Threads::Threads "${CMAKE_SOURCE_DIR}/glfw/lib-vc2019/glfw3.lib")
#	C:\Users\David Fussell\Source\Repos\wyrm\glfw\lib-vc2010-64\glfw3.lib
else()
	target_link_libraries(wyrm PRIVATE Threads::Threads)
endif()
