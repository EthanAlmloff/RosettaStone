# CMake version
cmake_minimum_required(VERSION 3.31.6 FATAL_ERROR)

# Include cmake modules
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/Builds/CMake")

# Declare project
project(RosettaStone)

if(MSVC)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>" CACHE STRING "" FORCE)
endif()

# Set RosettaStone's root directory
if (NOT ROSETTA_ROOT)
	set(ROSETTA_ROOT ${PROJECT_SOURCE_DIR})
endif()

# Set output directories
set(DEFAULT_CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_LIBRARY_OUTPUT_DIRECTORY})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)

# Set enable output of compile commands during generation
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Includes
include_directories(Includes)
include_directories(Extensions/RosettaPython/Includes)

# Dependencies
option(BUILD_TESTING "Build RosettaStone tests" ON)
if (BUILD_TESTING)
    find_package(doctest CONFIG REQUIRED)
endif()
find_package(effolkronium_random CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
option(BUILD_EXTENSIONS "Build RosettaStone command-line extensions" ON)
if (BUILD_EXTENSIONS)
    find_package(lyra CONFIG REQUIRED)
    find_package(pybind11 CONFIG REQUIRED)
endif()

# Compile options
include(Builds/CMake/CompileOptions.cmake)

# Build type - Release by default
if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Overrides
set(CMAKE_MACOSX_RPATH ON)

# Set resources
set(RESOURCES_DIR ${ROSETTA_ROOT}/Resources)

# When RosettaStone is configured from the HSBGML checkout, prefer the
# content-addressed Patch 36.4 snapshot checked in by the parent project.  A
# plain RosettaStone checkout still falls back to its historical resource;
# callers can always provide an explicit path with the cache variable below.
set(HSBGML_PINNED_CARDS_JSON
    "${CMAKE_CURRENT_SOURCE_DIR}/../../data/hearthstonejson/36.4/cards.json")
if (EXISTS "${HSBGML_PINNED_CARDS_JSON}")
    set(ROSETTA_BATTLEGROUNDS_CARDS_JSON_DEFAULT
        "${HSBGML_PINNED_CARDS_JSON}")
else ()
    set(ROSETTA_BATTLEGROUNDS_CARDS_JSON_DEFAULT
        "${RESOURCES_DIR}/cards.json")
endif ()
set(ROSETTA_BATTLEGROUNDS_CARDS_JSON
    "${ROSETTA_BATTLEGROUNDS_CARDS_JSON_DEFAULT}"
    CACHE FILEPATH
    "Explicit HearthstoneJSON snapshot used by the Battlegrounds loader (never latest)")
if (NOT EXISTS "${ROSETTA_BATTLEGROUNDS_CARDS_JSON}")
    message(FATAL_ERROR
        "ROSETTA_BATTLEGROUNDS_CARDS_JSON does not exist: ${ROSETTA_BATTLEGROUNDS_CARDS_JSON}")
endif()

# The parent checkout's default is required to be byte-identical to the
# pinned HearthstoneJSON build.  This catches a stale Resources/cards.json or
# a partially refreshed artifact at configure time, before any simulator
# library is compiled.  Standalone RosettaStone builds retain their normal
# resource fallback because the parent snapshot is not present there.
if (EXISTS "${HSBGML_PINNED_CARDS_JSON}")
    get_filename_component(configured_cards_real
        "${ROSETTA_BATTLEGROUNDS_CARDS_JSON}" REALPATH)
    get_filename_component(pinned_cards_real
        "${HSBGML_PINNED_CARDS_JSON}" REALPATH)
    if (NOT "${configured_cards_real}" STREQUAL "${pinned_cards_real}")
        message(FATAL_ERROR
            "HSBGML requires the pinned Patch 36.4 cards.json: "
            "${HSBGML_PINNED_CARDS_JSON}; got ${ROSETTA_BATTLEGROUNDS_CARDS_JSON}. "
            "Override only for an isolated non-production build after reviewing the pin.")
    endif()
    file(SHA256 "${ROSETTA_BATTLEGROUNDS_CARDS_JSON}" configured_cards_sha256)
    if (NOT "${configured_cards_sha256}" STREQUAL
        "00172dc60d55c34ff6c1417c329f1df645a705398964fb82bd089238dc3f3d02")
        message(FATAL_ERROR
            "Pinned Patch 36.4 cards.json has unexpected SHA-256: "
            "${configured_cards_sha256}")
    endif()
endif()
file(TO_CMAKE_PATH "${ROSETTA_BATTLEGROUNDS_CARDS_JSON}"
     ROSETTA_BATTLEGROUNDS_CARDS_JSON_CMAKE)
set(RESOURCES_JSONS
    ${RESOURCES_DIR}/cards.collectible.json
    ${ROSETTA_BATTLEGROUNDS_CARDS_JSON})

# Project modules
add_subdirectory(Sources/Rosetta)
if (BUILD_TESTING)
    add_subdirectory(Tests/UnitTests)
endif()
if (BUILD_EXTENSIONS)
    add_subdirectory(Extensions/RosettaConsole)
    add_subdirectory(Extensions/RosettaTool)
endif()

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Builds/setup.py.in ${CMAKE_CURRENT_SOURCE_DIR}/setup.py)

# Code coverage - Debug only
# NOTE: Code coverage results with an optimized (non-Debug) build may be misleading
option(BUILD_COVERAGE "Build code coverage" OFF)
if (CMAKE_BUILD_TYPE MATCHES Debug AND CMAKE_COMPILER_IS_GNUCXX AND BUILD_COVERAGE)
    include(CodeCoverage)
    setup_target_for_coverage(${PROJECT_NAME}_coverage UnitTests coverage)
endif()

if (BUILD_FROM_PIP)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${DEFAULT_CMAKE_LIBRARY_OUTPUT_DIRECTORY})
endif()
add_subdirectory(Extensions/RosettaPython)
