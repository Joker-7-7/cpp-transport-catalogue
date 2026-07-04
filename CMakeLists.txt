cmake_minimum_required(VERSION 3.19)
project(cpp-transport-catalogue LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON) 

file(GLOB_RECURSE TRANSPORT_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/transport-catalogue/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/transport-catalogue/*.h
)
add_executable(cpp-transport-catalogue
    ${TRANSPORT_SOURCES}
)
