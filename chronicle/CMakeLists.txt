# Chronicle Opcode Library
# Builds a minimal shared library of Csound opcodes for Chronicle
# This is LGPL-licensed (same as Csound)

cmake_minimum_required(VERSION 3.15)
project(CsoundOpcodes VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# Match Csound's USE_DOUBLE for precision
add_definitions(-DUSE_DOUBLE)
add_definitions(-D__BUILDING_LIBCSOUND)
add_definitions(-DCSOUND_OPCODES_BUILDING)
add_definitions(-DCHRONICLE)
add_definitions(-DCHRONICLE_NO_LPC)
add_definitions(-DCHRONICLE_NO_PVOC)

# Source directories
set(CSOUND_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/..)
set(CHRONICLE_CORE ${CMAKE_CURRENT_SOURCE_DIR}/../../../packages/chronicle-core)

# Include directories
# CRITICAL: Chronicle's shim MUST come first to override Csound headers
# This provides stub implementations for csoundLocalizeString, etc.
set(CSOUND_INCLUDES
    ${CHRONICLE_CORE}/shim       # Chronicle's shim (CsoundShim.hpp, ugens2.h, ugens5.h)
    ${CMAKE_CURRENT_SOURCE_DIR}  # Our chronicle dir (version.h, fgens_chronicle.h)
    ${CSOUND_ROOT}/include
    ${CSOUND_ROOT}/H
    ${CSOUND_ROOT}
)

# Core opcode sources
set(OPCODE_SOURCES
    # Opcode registry - SSOT for all opcode metadata
    ${CMAKE_CURRENT_SOURCE_DIR}/opcode_registry.c
    # GEN routines for wavetables
    ${CMAKE_CURRENT_SOURCE_DIR}/fgens_chronicle.c
    # Oscillators: phasor, oscil, etc.
    ${CSOUND_ROOT}/OOps/ugens2.c
    # Filters: tone, port, reson, etc.
    ${CSOUND_ROOT}/OOps/ugens5.c
    # New filters: moogladder, statevar, etc.
    ${CSOUND_ROOT}/Opcodes/newfils.c
    # Band-limited oscillators: vco2
    ${CSOUND_ROOT}/Opcodes/oscbnk.c
)

# Platform-specific build configuration
if(EMSCRIPTEN)
    # WASM side module - must use add_executable for Emscripten dynamic linking
    add_executable(csound_opcodes ${OPCODE_SOURCES})
    target_include_directories(csound_opcodes PRIVATE ${CSOUND_INCLUDES})

    set_target_properties(csound_opcodes PROPERTIES
        SUFFIX ".wasm"
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
    )
    # SIDE_MODULE=1 exports everything (larger but simpler)
    # We need all symbols since the main module calls them via dlsym
    target_link_options(csound_opcodes PRIVATE
        -sSIDE_MODULE=1
        -O3
    )
    target_compile_options(csound_opcodes PRIVATE -fPIC -O3)
else()
    # Native shared library (.dylib / .so)
    add_library(csound_opcodes SHARED ${OPCODE_SOURCES})
    target_include_directories(csound_opcodes PRIVATE ${CSOUND_INCLUDES})

    set_target_properties(csound_opcodes PROPERTIES
        VERSION ${PROJECT_VERSION}
        SOVERSION 1
        C_VISIBILITY_PRESET default
        CXX_VISIBILITY_PRESET default
    )

    if(APPLE)
        set_target_properties(csound_opcodes PROPERTIES
            INSTALL_RPATH "@loader_path"
        )
    elseif(UNIX)
        set_target_properties(csound_opcodes PROPERTIES
            INSTALL_RPATH "$ORIGIN"
        )
    endif()
endif()

# Install
install(TARGETS csound_opcodes
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

install(FILES
    opcode_registry.h
    DESTINATION include
)
