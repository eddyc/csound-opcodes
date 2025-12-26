/*
    csound_opcodes.h

    Chronicle Opcode Library - Minimal Csound opcode exports
    This is LGPL-licensed (same as Csound)

    Copyright (C) 2024

    This file provides a clean C API for accessing Csound opcodes
    from Chronicle without needing the full Csound runtime.
*/

#ifndef CSOUND_OPCODES_H
#define CSOUND_OPCODES_H

#ifdef __cplusplus
extern "C" {
#endif

/* Include Csound types */
#include "csoundCore.h"
#include "H/ugens5.h"
#include "H/ugens2.h"

/* ============================================================
 * Filter Opcodes (from ugens5.c)
 * ============================================================ */

/* PORT - portamento/glide */
int32_t porset(CSOUND *csound, PORT *p);
int32_t port(CSOUND *csound, PORT *p);
int32_t kport(CSOUND *csound, PORT *p);

/* TONE - first-order lowpass filter */
int32_t tonset(CSOUND *csound, TONE *p);
int32_t ktone(CSOUND *csound, TONE *p);
int32_t tone(CSOUND *csound, TONE *p);

/* TONEX - cascaded first-order lowpass filters */
int32_t tonsetx(CSOUND *csound, TONEX *p);
int32_t tonex(CSOUND *csound, TONEX *p);

/* ATONE - first-order highpass filter */
int32_t katone(CSOUND *csound, TONE *p);
int32_t atone(CSOUND *csound, TONE *p);
int32_t atonex(CSOUND *csound, TONEX *p);

/* RESON - second-order resonant bandpass filter */
int32_t rsnset(CSOUND *csound, RESON *p);
int32_t krsnset(CSOUND *csound, RESON *p);
int32_t kreson(CSOUND *csound, RESON *p);
int32_t reson(CSOUND *csound, RESON *p);

/* RESONX - cascaded resonant filters */
int32_t rsnsetx(CSOUND *csound, RESONX *p);
int32_t resonx(CSOUND *csound, RESONX *p);

/* ARESON - anti-resonant (notch) filter */
int32_t kareson(CSOUND *csound, RESON *p);
int32_t areson(CSOUND *csound, RESON *p);

/* RMS - RMS measurement */
int32_t rmsset(CSOUND *csound, RMS *p);
int32_t rms(CSOUND *csound, RMS *p);

/* GAIN - automatic gain control */
int32_t gainset(CSOUND *csound, GAIN *p);
int32_t gain(CSOUND *csound, GAIN *p);

/* BALANCE - balance two signals by RMS */
int32_t balnset(CSOUND *csound, BALANCE *p);
int32_t balance(CSOUND *csound, BALANCE *p);
int32_t balance2(CSOUND *csound, BALANCE *p);

/* LIMIT - signal limiter */
int32_t klimit(CSOUND *csound, LIMIT *p);
int32_t limit(CSOUND *csound, LIMIT *p);

/* ============================================================
 * Oscillator Opcodes (from ugens2.c)
 * ============================================================ */

/* PHASOR - phase accumulator */
int32_t phsset(CSOUND *csound, PHSOR *p);
int32_t kphsor(CSOUND *csound, PHSOR *p);
int32_t phsor(CSOUND *csound, PHSOR *p);

/* TABLE - table lookup */
int32_t tarone(CSOUND *csound, TARONE *p);
int32_t tarone_k(CSOUND *csound, TARONE *p);
int32_t itablew(CSOUND *csound, TABLEW *p);
int32_t tblsetw(CSOUND *csound, TABLEW *p);
int32_t ktablew(CSOUND *csound, TABLEW *p);
int32_t tablew(CSOUND *csound, TABLEW *p);

/* OSCIL - table-based oscillator */
int32_t oscset(CSOUND *csound, OSC *p);
int32_t koscil(CSOUND *csound, OSC *p);
int32_t osckk(CSOUND *csound, OSC *p);
int32_t oscka(CSOUND *csound, OSC *p);
int32_t oscak(CSOUND *csound, OSC *p);
int32_t oscaa(CSOUND *csound, OSC *p);

#ifdef __cplusplus
}
#endif

#endif /* CSOUND_OPCODES_H */
