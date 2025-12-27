/*
    fgens_chronicle.h

    Chronicle-specific exports for Csound GEN routines.
    This header exposes gen05, gen07, gen10 for direct use by Chronicle.

    LGPL licensed (same as Csound).
*/

#ifndef FGENS_CHRONICLE_H
#define FGENS_CHRONICLE_H

#include "csoundCore.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * GEN routines exposed for Chronicle's table generation.
 * These use the standard Csound FGDATA/FUNC interface.
 *
 * Arguments are passed via FGDATA->e.p[] array:
 *   p[4] = first gen argument (for gen10: amplitude of harmonic 1)
 *   p[5] = second gen argument
 *   etc.
 * FGDATA->e.pcnt = total parameter count (4 + number of gen args)
 */

/* GEN05 - Exponential curve segments */
int chronicle_gen05(FGDATA *ff, FUNC *ftp);

/* GEN07 - Straight line segments */
int chronicle_gen07(FGDATA *ff, FUNC *ftp);

/* GEN10 - Composite sine waves (additive synthesis) */
int chronicle_gen10(FGDATA *ff, FUNC *ftp);

#ifdef __cplusplus
}
#endif

#endif /* FGENS_CHRONICLE_H */
