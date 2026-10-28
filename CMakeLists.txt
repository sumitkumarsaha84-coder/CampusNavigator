cmake_minimum_required(VERSION 3.15)
project(CampusNavigator
    VERSION 1.0.0
    DESCRIPTION "Campus Navigator - Smart Campus Route Finder (ADSA Semester Project)"
    LANGUAGES CXX
)

# Standard C++17 configuration
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Compiler warnings and optimization flags
if(MSVC)
    add_compile_options(/W4 /permissive- /utf-8 /O2)
else()
    add_compile_options(-Wall -Wextra -Wpedantic -Wconversion -O2)
endif()

# Include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# Core library sources
set(CAMPUS_CORE_SOURCES
    src/CampusGraph.cpp
    src/PathFinder.cpp
    src/CampusData.cpp
    src/TerminalUI.cpp
    src/HttpServer.cpp
)

# Main interactive console application and REST server
add_executable(CampusNavigator
    src/main.cpp
    ${CAMPUS_CORE_SOURCES}
)

# Automated unit and integration test suite
add_executable(CampusNavigatorTests
    tests/test_main.cpp
    ${CAMPUS_CORE_SOURCES}
)

# Windows Sockets linking
if(WIN32)
    target_link_libraries(CampusNavigator PRIVATE ws2_32)
    target_link_libraries(CampusNavigatorTests PRIVATE ws2_32)
endif()

# Enable CTest integration
enable_testing()
add_test(
    NAME AlgorithmVerification
    COMMAND CampusNavigatorTests
)
