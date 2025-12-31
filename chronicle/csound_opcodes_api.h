/*
    csound_opcodes_api.h

    Chronicle Opcode Library - Dynamic Loading API
    This is LGPL-licensed (same as Csound)

    Copyright (C) 2024

    This header defines the public C API for loading Csound opcodes
    via dlopen at runtime. Chronicle links dynamically to this library
    for LGPL compliance.
*/

#ifndef CSOUND_OPCODES_API_H
#define CSOUND_OPCODES_API_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Visibility macro for shared library exports */
#if defined(_WIN32) || defined(__CYGWIN__)
  #ifdef CSOUND_OPCODES_BUILDING
    #define CSOUND_API __declspec(dllexport)
  #else
    #define CSOUND_API __declspec(dllimport)
  #endif
#else
  #define CSOUND_API __attribute__((visibility("default")))
#endif

/* Function pointer types (matches Chronicle's InitFunc/PerfFunc) */
typedef int32_t (*CsoundOpcodeInit)(void* csound, void* data);
typedef int32_t (*CsoundOpcodePerf)(void* csound, void* data);

/**
 * Opcode entry describing a single opcode
 */
typedef struct {
    const char* name;           /* Opcode name (e.g., "phasor") */
    size_t structSize;          /* sizeof the opcode struct */
    size_t opdsOffset;          /* offsetof(Struct, h) - OPDS header */
    size_t outputOffset;        /* offsetof(Struct, output) */
    size_t inputOffsets[8];     /* Input field offsets (0-terminated) */
    CsoundOpcodeInit init;      /* Init function (may be NULL) */
    CsoundOpcodePerf perf;      /* Performance function */
} CsoundOpcodeEntry;

/**
 * Get all available opcodes
 * Returns a NULL-terminated array of opcode entries
 */
CSOUND_API const CsoundOpcodeEntry* csound_get_opcodes(void);

/**
 * Get opcode count (not including NULL terminator)
 */
CSOUND_API int csound_get_opcode_count(void);

/**
 * Get library version string
 */
CSOUND_API const char* csound_get_version(void);

#ifdef __cplusplus
}
#endif

#endif /* CSOUND_OPCODES_API_H */
