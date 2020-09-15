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


add_executable(wyrm ${SOURCES})
target_link_libraries(wyrm PRIVATE Threads::Threads)
