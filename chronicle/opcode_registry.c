/*
    opcode_registry.c - Csound Opcode Registry Implementation

    This is LGPL-licensed (same as Csound)
    Copyright (C) 2024

    ============================================================
    SINGLE SOURCE OF TRUTH FOR CSOUND OPCODES
    ============================================================

    To add a new opcode:
    1. Add one line to the 'opcodes' array below
    2. Rebuild libcsound-opcodes
    3. Done - Chronicle picks it up automatically via dlopen

    No other files need to be modified!
*/

#include "opcode_registry.h"

/* Use Chronicle's shim for type definitions */
#include "CsoundShim.hpp"
#include "ugens2.h"
#include "ugens5.h"
#include "newfils.h"
#include "oscbnk.h"

/* Forward declarations of opcode functions we use */
int32_t phsset(CSOUND *csound, PHSOR *p);
int32_t phsor(CSOUND *csound, PHSOR *p);
int32_t oscset(CSOUND *csound, OSC *p);
int32_t osckk(CSOUND *csound, OSC *p);
int32_t tonset(CSOUND *csound, TONE *p);
int32_t tone(CSOUND *csound, TONE *p);
int32_t porset(CSOUND *csound, PORT *p);
int32_t kport(CSOUND *csound, PORT *p);
int32_t moogladder_init(CSOUND *csound, moogladder *p);
int32_t moogladder_process(CSOUND *csound, moogladder *p);
int32_t vco2set(CSOUND *csound, VCO2 *p);
int32_t vco2(CSOUND *csound, VCO2 *p);

/* Version string */
static const char* LIB_VERSION = "1.0.0";

/* ============================================================
 * OPCODE REGISTRY - Add new opcodes here!
 *
 * Format:
 *   CSOUND_OP(name, Struct, outputField, initFunc, perfFunc, input1, input2, ...)
 *
 * Where:
 *   name        - The opcode name as used in Chronicle DSL
 *   Struct      - The C struct type (e.g., PHSOR, TONE)
 *   outputField - The struct field containing output pointer (e.g., sr, ar, kr)
 *   initFunc    - The init function (or NULL)
 *   perfFunc    - The performance function
 *   inputs...   - The input field names in order
 * ============================================================ */

static const CsoundOpcodeEntry opcodes[] = {
    /* Oscillators (from ugens2.c) */
    CSOUND_OP(phasor, PHSOR, sr,  phsset, phsor,   xcps, iphs),
    CSOUND_OP(oscil,  OSC,   sr,  oscset, osckk,   xamp, xcps, ifn, iphs),

    /* Filters (from ugens5.c) */
    CSOUND_OP(lpf,    TONE,  ar,  tonset, tone,    asig, khp, istor),
    CSOUND_OP(port,   PORT,  kr,  porset, kport,   ksig, ihtim, isig),

    /* New filters (from newfils.c) */
    CSOUND_OP(moogladder, moogladder, out, moogladder_init, moogladder_process, in, freq, res, istor),

    /* Band-limited oscillators (from oscbnk.c) */
    CSOUND_OP(vco2, VCO2, ar, vco2set, vco2, kamp, kcps, imode, kpw, kphs, inyx),

    /* Terminator - must be last */
    { NULL, 0, 0, 0, {0}, NULL, NULL }
};

/* ============================================================
 * Public API
 * ============================================================ */

CSOUND_API const CsoundOpcodeEntry* csound_get_opcodes(void) {
    return opcodes;
}

CSOUND_API const char* csound_get_version(void) {
    return LIB_VERSION;
}
