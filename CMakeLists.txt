cmake_minimum_required(VERSION 3.6)

project(seabattle)

set(CMAKE_CXX_STANDARD 11)

add_custom_target(
   odoors
   COMMAND make
   WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/odoors
)

add_executable(seabattle main.c)
include_directories(${CMAKE_SOURCE_DIR}/odoors)
TARGET_LINK_LIBRARIES(seabattle ${CMAKE_SOURCE_DIR}/odoors/libs-${CMAKE_SYSTEM_NAME}/libODoors.a)
add_dependencies(seabattle odoors)
