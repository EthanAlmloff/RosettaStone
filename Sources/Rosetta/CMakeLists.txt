# Target name
set(target RosettaStone)

# Define
set(root_dir ${CMAKE_CURRENT_SOURCE_DIR}/../..)

# Includes
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/../Includes
)

# Sources
file(GLOB header_dir
    ${root_dir}/Includes/Rosetta)

file(GLOB_RECURSE headers CONFIGURE_DEPENDS
    ${header_dir}/*.hpp)

file(GLOB_RECURSE sources CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

# GeneratedBehaviorCards.cpp is a generator staging artifact.  It mirrors the
# declarative rows while a review is in progress, but the live CardDefs
# registry is owned by the canonical GeneratedBehaviorMappings.cpp unit.  Do
# not let the recursive source glob link both definitions of
# GeneratedBehaviorMappings::AddAll (MSVC reports LNK4006/LNK2005).
list(FILTER sources EXCLUDE REGEX "/GeneratedBehaviorCards\\.cpp$")

# Custom-build event
if (CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    set(rosettastone_header_gen_py ${root_dir}/Scripts/header_gen.py)
    add_custom_command(
        COMMAND python ${rosettastone_header_gen_py}
        DEPENDS ${headers}
        OUTPUT ${root_dir}/Includes/RosettaStone.hpp
    )
    add_custom_target(rosettastone_header_gen_py ALL
        DEPENDS ${root_dir}/Includes/RosettaStone.hpp)
else ()
    set(rosettastone_header_gen_py ${root_dir}/Scripts/header_gen.py)
    add_custom_command(
        COMMAND python3 ${rosettastone_header_gen_py}
        DEPENDS ${headers}
        OUTPUT ${root_dir}/Includes/RosettaStone.hpp
    )
    add_custom_target(rosettastone_header_gen_py ALL
        DEPENDS ${root_dir}/Includes/RosettaStone.hpp)
endif ()

# Build library
add_library(${target}
    ${sources})
add_dependencies(${target} rosettastone_header_gen_py)

# Project options
set_target_properties(${target}
PROPERTIES
    WINDOWS_EXPORT_ALL_SYMBOLS  true
    ${DEFAULT_PROJECT_OPTIONS}
)

# Compile options
target_compile_options(${target}
    PRIVATE

    PUBLIC
    ${DEFAULT_COMPILE_OPTIONS}

    INTERFACE
)
target_compile_definitions(${target}
    PRIVATE
    RESOURCES_DIR="${ROSETTA_ROOT}/Resources/"
    ROSETTA_BATTLEGROUNDS_CARDS_JSON="${ROSETTA_BATTLEGROUNDS_CARDS_JSON_CMAKE}"
)

target_link_libraries(${target}
    PRIVATE

    PUBLIC
    ${DEFAULT_LINKER_OPTIONS}
    effolkronium_random
    nlohmann_json::nlohmann_json

    INTERFACE
)

# Install
install(TARGETS ${target} DESTINATION lib)
install(DIRECTORY ${header_dir} DESTINATION include)
