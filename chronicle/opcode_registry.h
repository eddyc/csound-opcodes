/*
    opcode_registry.h - Csound Opcode Registry API

    This is LGPL-licensed (same as Csound)
    Copyright (C) 2024

    Provides a single-source-of-truth registry for Csound opcodes.
    Chronicle loads this table via dlopen at runtime.
*/

#ifndef OPCODE_REGISTRY_H
#define OPCODE_REGISTRY_H

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

/* Maximum number of inputs per opcode */
#define OPCODE_MAX_INPUTS 8

/* Function pointer types */
typedef int32_t (*OpcodeFunc)(void* csound, void* data);

/**
 * Opcode entry with complete metadata
 * All offsets are computed at compile time via offsetof()
 */
typedef struct {
    const char* name;                       /* Opcode name (e.g., "phasor") */
    size_t structSize;                      /* sizeof(OpcodeStruct) */
    size_t opdsOffset;                      /* offsetof(Struct, h) - OPDS header */
    size_t outputOffset;                    /* offsetof(Struct, outputField) */
    size_t inputOffsets[OPCODE_MAX_INPUTS]; /* Input field offsets (0-terminated) */
    OpcodeFunc init;                        /* Init function (may be NULL) */
    OpcodeFunc perf;                        /* Performance function */
} CsoundOpcodeEntry;

/**
 * Get all available opcodes
 * Returns a NULL-terminated array of opcode entries
 */
CSOUND_API const CsoundOpcodeEntry* csound_get_opcodes(void);

/**
 * Get library version string
 */
CSOUND_API const char* csound_get_version(void);

/* ============================================================
 * CSOUND_OP Macro - Single source of truth for opcode definitions
 *
 * Usage:
 *   CSOUND_OP(name, Struct, outField, initFn, perfFn, in1, in2, ...)
 *
 * Example:
 *   CSOUND_OP(phasor, PHSOR, sr, phsset, phsor, xcps, iphs)
 *
 * This generates all the metadata Chronicle needs:
 *   - struct size
 *   - OPDS offset
 *   - output field offset
 *   - input field offsets
 *   - init/perf function pointers
 * ============================================================ */

/* Helper macros for counting and expanding variadic args */
#define OFFSET(S, f) offsetof(S, f)

/* Expand up to 8 input offsets, padding with 0s */
#define INPUTS_0(S)                          {0}
#define INPUTS_1(S, a)                       {OFFSET(S,a), 0}
#define INPUTS_2(S, a,b)                     {OFFSET(S,a), OFFSET(S,b), 0}
#define INPUTS_3(S, a,b,c)                   {OFFSET(S,a), OFFSET(S,b), OFFSET(S,c), 0}
#define INPUTS_4(S, a,b,c,d)                 {OFFSET(S,a), OFFSET(S,b), OFFSET(S,c), OFFSET(S,d), 0}
#define INPUTS_5(S, a,b,c,d,e)               {OFFSET(S,a), OFFSET(S,b), OFFSET(S,c), OFFSET(S,d), OFFSET(S,e), 0}
#define INPUTS_6(S, a,b,c,d,e,f)             {OFFSET(S,a), OFFSET(S,b), OFFSET(S,c), OFFSET(S,d), OFFSET(S,e), OFFSET(S,f), 0}
#define INPUTS_7(S, a,b,c,d,e,f,g)           {OFFSET(S,a), OFFSET(S,b), OFFSET(S,c), OFFSET(S,d), OFFSET(S,e), OFFSET(S,f), OFFSET(S,g), 0}
#define INPUTS_8(S, a,b,c,d,e,f,g,h)         {OFFSET(S,a), OFFSET(S,b), OFFSET(S,c), OFFSET(S,d), OFFSET(S,e), OFFSET(S,f), OFFSET(S,g), OFFSET(S,h)}

/* Count arguments macro */
#define COUNT_ARGS_IMPL(_1,_2,_3,_4,_5,_6,_7,_8,N,...) N
#define COUNT_ARGS(...) COUNT_ARGS_IMPL(__VA_ARGS__, 8, 7, 6, 5, 4, 3, 2, 1, 0)

/* Select the right INPUTS_N macro based on arg count */
#define INPUTS_SELECT(S, N, ...) INPUTS_##N(S, __VA_ARGS__)
#define INPUTS_EXPAND(S, N, ...) INPUTS_SELECT(S, N, __VA_ARGS__)

/* Main macro - generates a CsoundOpcodeEntry */
#define CSOUND_OP(name, Struct, outField, initFn, perfFn, ...) \
    { #name, sizeof(Struct), \
      offsetof(Struct, h), \
      offsetof(Struct, outField), \
      INPUTS_EXPAND(Struct, COUNT_ARGS(__VA_ARGS__), __VA_ARGS__), \
      (OpcodeFunc)initFn, \
      (OpcodeFunc)perfFn }

/* Variant for opcodes with no inputs */
#define CSOUND_OP_NOINPUT(name, Struct, outField, initFn, perfFn) \
    { #name, sizeof(Struct), \
      offsetof(Struct, h), \
      offsetof(Struct, outField), \
      {0}, \
      (OpcodeFunc)initFn, \
      (OpcodeFunc)perfFn }

#ifdef __cplusplus
}
#endif

#endif /* OPCODE_REGISTRY_H */
