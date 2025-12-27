/*
    fgens_chronicle.c

    Chronicle-specific GEN routines extracted from Csound's Engine/fgens.c.
    These use our minimal shim types (FGDATA_SHIM, etc.) to avoid
    pulling in complex Csound dependencies.

    Copyright (C) 1991, 1994, 1995, 1998, 2000, 2004
                  Barry Vercoe, John ffitch, Paris Smaragdis,
                  Gabriel Maldonado, Richard Karpen, Greg Sullivan,
                  Pete Moss, Istvan Varga, Victor Lazzarini

    LGPL licensed (same as Csound).
*/

#include "csoundCore.h"
#include "fgens_chronicle.h"
#include <math.h>

#ifndef POWER
#define POWER(x,y)  pow((double)(x), (double)(y))
#endif

/*
 * GEN05 - Exponential curve segments
 * args = [value1, samples1, value2, samples2, value3, ...]
 * Adapted from Csound's Engine/fgens.c gen05()
 */
int chronicle_gen05(FGDATA *ff, FUNC *ftp)
{
    int     nsegs, seglen;
    MYFLT   *valp, *fp, *finp;
    MYFLT   amp1, mult;
    int nsw = 1;

    if ((nsegs = (ff->e.pcnt - 5) >> 1) <= 0)    /* nsegs = nargs-1 / 2 */
        return OK;
    valp = &ff->e.p[5];
    fp = ftp->ftable;
    finp = fp + ff->flen;
    if (*valp == 0) return NOTOK;  /* First value must be non-zero */
    do {
        amp1 = *valp++;
        if (nsw && valp > &ff->e.p[PMAX]) {
            valp = &(ff->e.c.extra[1]);
            nsw = 0;
        }
        if (!(seglen = (int)*valp++)) {
            if (nsw && valp > &ff->e.p[PMAX]) {
                valp = &(ff->e.c.extra[1]);
                nsw = 0;
            }
            continue;
        }
        if (nsw && valp > &ff->e.p[PMAX]) {
            valp = &(ff->e.c.extra[1]);
            nsw = 0;
        }
        if (seglen < 0) return NOTOK;
        if ((mult = *valp / amp1) <= 0) return NOTOK;
        mult = POWER(mult, FL(1.0) / seglen);
        while (seglen--) {
            *fp++ = amp1;
            amp1 *= mult;
            if (fp > finp)
                return OK;
        }
    } while (--nsegs);
    if (fp == finp)                 /* if 2**n pnts, add guardpt */
        *fp = amp1;
    return OK;
}

/*
 * GEN07 - Straight line segments
 * args = [value1, samples1, value2, samples2, value3, ...]
 * Adapted from Csound's Engine/fgens.c gen07()
 */
int chronicle_gen07(FGDATA *ff, FUNC *ftp)
{
    int     nsegs, seglen;
    MYFLT   *valp, *fp, *finp;
    MYFLT   amp1, incr;

    if ((nsegs = (ff->e.pcnt - 5) >> 1) <= 0)         /* nsegs = nargs-1 / 2 */
        return OK;
    valp = &ff->e.p[5];
    fp = ftp->ftable;
    finp = fp + ff->flen;
    do {
        amp1 = *valp++;
        if (!(seglen = (int)*valp++)) continue;
        if (seglen < 0) return NOTOK;
        incr = (*valp - amp1) / seglen;
        while (seglen--) {
            *fp++ = amp1;
            amp1 += incr;
            if (fp > finp)
                return OK;
        }
    } while (--nsegs);
    if (fp == finp)                 /* if 2**n pnts, add guardpt */
        *fp = amp1;
    return OK;
}

/*
 * GEN10 - Composite sine waves (additive synthesis)
 * args = [amp1, amp2, amp3, ...] where each is the relative strength of harmonic 1, 2, 3...
 * Adapted from Csound's Engine/fgens.c gen10()
 */
int chronicle_gen10(FGDATA *ff, FUNC *ftp)
{
    int32_t phs, hcnt;
    MYFLT   amp, *fp, *finp;
    int32_t flen = ff->flen;
    double  tpdlen = TWOPI / (double)flen;

    hcnt = ff->e.pcnt - 4;                              /* hcnt is nargs */
    finp = &ftp->ftable[flen];
    do {
        MYFLT *valp = (hcnt + 4 >= PMAX ? &ff->e.c.extra[hcnt + 5 - PMAX] :
                                          &ff->e.p[hcnt + 4]);
        if ((amp = *valp) != FL(0.0))         /* for non-0 amps, */
            for (phs = 0, fp = ftp->ftable; fp <= finp; fp++) {
                *fp += (MYFLT)sin(phs * tpdlen) * amp;    /* accum sin pts */
                phs += hcnt;                              /* phsinc is hno */
                phs %= flen;
            }
    } while (--hcnt);

    return OK;
}
