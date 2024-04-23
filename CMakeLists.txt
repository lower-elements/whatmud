cmake_minimum_required(VERSION 3.25)

project(WhatMUD
    VERSION 0.1.0
    DESCRIPTION "A scriptable MUD engine"
    LANGUAGES CXX
)

# Global project options
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

# Warnings
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang|AppleClang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic")
elseif(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
endif()

# Dependencies
include(FetchContent)
find_package(PkgConfig)

FetchContent_Declare(
  fmt
  GIT_REPOSITORY "https://github.com/fmtlib/fmt"
  GIT_TAG "10.2.1"
  FIND_PACKAGE_ARGS NAMES fmt
)

pkg_check_modules(telnet libtelnet)
if(NOT telnet_FOUND)
FetchContent_Declare(
    libtelnet
    GIT_REPOSITORY "https://github.com/seanmiddleditch/libtelnet"
    GIT_TAG "5f5ecee776b9bdaa4e981e5f807079a9c79d633e"
    )
endif()

pkg_check_modules(uv libuv)
if(NOT uv_FOUND)
FetchContent_Declare(
    libuv
    GIT_REPOSITORY "https://github.com/libuv/libuv"
    GIT_TAG "v1.48.0"
    )
endif()

FetchContent_Declare(
    lua
    GIT_REPOSITORY "https://github.com/marovira/lua"
    GIT_TAG "5.4.4"
    FIND_PACKAGE_ARGS
)

set(SPDLOG_FMT_EXTERNAL ON)
FetchContent_Declare(
    spdlog
    GIT_REPOSITORY "https://github.com/gabime/spdlog"
    GIT_TAG "v1.13.0"
    FIND_PACKAGE_ARGS NAMES spdlog
    )

FetchContent_Declare(
    SQLite3
    GIT_REPOSITORY "https://github.com/azadkuh/sqlite-amalgamation"
    GIT_TAG "3.38.2"
    FIND_PACKAGE_ARGS
    )

# Much fiddling with CMake to make sure deps work whether they're found or built
FetchContent_MakeAvailable(fmt Lua spdlog SQLite3)
if(NOT telnet_FOUND)
    FetchContent_MakeAvailable(libtelnet)
    add_library(telnet ALIAS libtelnet)
endif()
if(NOT uv_FOUND)
    FetchContent_MakeAvailable(libuv)
endif()
if (LUA_FOUND)
    add_library(lua_lib INTERFACE)
    target_include_directories(lua_lib INTERFACE ${LUA_INCLUDE_DIR})
    target_link_libraries(lua_lib INTERFACE ${LUA_LIBRARIES})
endif()
if (SQLite3_FOUND)
    add_library(SQLite3 ALIAS SQLite::SQLite3)
endif()

add_executable(whatmud
    src/engine.cpp
    src/lua/error.cpp
    src/lua/stack.cpp
    src/lua/state.cpp
    src/lua/table_view.cpp
    src/main.cpp
    src/uv/error.cpp
    )
target_include_directories(whatmud PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(whatmud PRIVATE fmt telnet lua_lib spdlog::spdlog SQLite3 uv)
install(TARGETS whatmud DESTINATION bin)
