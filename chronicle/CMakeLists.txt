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

# Source directory
set(CSOUND_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/..)

# Include directories
# Note: Our chronicle dir comes first so our version.h is found before
# looking for the missing one in include/
set(CSOUND_INCLUDES
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CSOUND_ROOT}/include
    ${CSOUND_ROOT}/H
    ${CSOUND_ROOT}
)

# Core opcode sources we need for filters and oscillators
# Note: We use our own ugens5_filters.c which excludes LPC opcodes
set(OPCODE_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/ugens5_filters.c
    ${CSOUND_ROOT}/OOps/ugens2.c
)

# Create shared library
add_library(csound_opcodes SHARED ${OPCODE_SOURCES})

target_include_directories(csound_opcodes PRIVATE ${CSOUND_INCLUDES})

# Export symbols
set_target_properties(csound_opcodes PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION 1
    C_VISIBILITY_PRESET default
)

# Platform-specific settings
if(APPLE)
    set_target_properties(csound_opcodes PROPERTIES
        INSTALL_RPATH "@loader_path"
    )
elseif(UNIX)
    set_target_properties(csound_opcodes PROPERTIES
        INSTALL_RPATH "$ORIGIN"
    )
endif()

# Emscripten side module build
if(EMSCRIPTEN)
    set_target_properties(csound_opcodes PROPERTIES
        SUFFIX ".wasm"
    )
    target_link_options(csound_opcodes PRIVATE
        -sSIDE_MODULE=1
        -sEXPORT_ALL=1
        -O3
    )
endif()

# Install
install(TARGETS csound_opcodes
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

install(FILES
    csound_opcodes.h
    DESTINATION include
)
