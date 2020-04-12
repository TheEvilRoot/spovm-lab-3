cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 14)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_HOME_DIRECTORY}/build)

set(MODULES server client)

project(0003)

add_subdirectory(server)
add_subdirectory(client)

add_custom_target(${PROJECT_NAME} DEPENDS ${MODULES})