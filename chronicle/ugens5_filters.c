/*
    ugens5_filters.c:

    Filter opcodes extracted from Csound's OOps/ugens5.c
    Excludes LPC analysis opcodes to avoid complex dependencies.

    Copyright (C) 1991 Barry Vercoe, John ffitch, Gabriel Maldonado

    This file is part of Csound.

    The Csound Library is free software; you can redistribute it
    and/or modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    Csound is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with Csound; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
    02110-1301 USA
*/

#include "csoundCore.h"
#include "ugens5.h"
#include <math.h>

/* PORT - portamento/glide */

int32_t porset(CSOUND *csound, PORT *p)
{
   IGN(csound);
    p->c2 = pow(0.5, (double)CS_ONEDKR / *p->ihtim);
    p->c1 = 1.0 - p->c2;
    if (LIKELY(*p->isig >= FL(0.0)))
      p->yt1 = (double)(*p->isig);
    p->ihtim_old = *p->ihtim;
    return OK;
}

int32_t port(CSOUND *csound, PORT *p)
{
    IGN(csound);
    p->yt1 = p->c1 * (double)*p->ksig + p->c2 * p->yt1;
    *p->kr = (MYFLT)p->yt1;
    return OK;
}

int32_t kport(CSOUND *csound, PORT *p)
{
    IGN(csound);
    if (p->ihtim_old != *p->ihtim) {
      p->c2 = pow(0.5, (double)CS_ONEDKR / *p->ihtim);
      p->c1 = 1.0 - p->c2;
      p->ihtim_old = *p->ihtim;
    }
    p->yt1 = p->c1 * (double)*p->ksig + p->c2 * p->yt1;
    *p->kr = (MYFLT)p->yt1;
    return OK;
}

/* TONE - first-order lowpass filter */

int32_t tonset(CSOUND *csound, TONE *p)
{
    double b;
    p->prvhp = (double)*p->khp;
    b = 2.0 - cos((double)(p->prvhp * csound->tpidsr));
    p->c2 = b - sqrt(b * b - 1.0);
    p->c1 = 1.0 - p->c2;

    if (LIKELY(!(*p->istor)))
      p->yt1 = 0.0;
    return OK;
}

int32_t ktone(CSOUND *csound, TONE *p)
{
    IGN(csound);
    double      c1 = p->c1, c2 = p->c2;
    double      yt1 = p->yt1;

    if (*p->khp != (MYFLT)p->prvhp) {
      double b;
      p->prvhp = (double)*p->khp;
      b = 2.0 - cos((double)(p->prvhp * CS_ONEDKR *TWOPI));
      p->c2 = c2 = b - sqrt(b * b - 1.0);
      p->c1 = c1 = 1.0 - c2;
    }
    yt1 = c1 * (double)(*p->asig) + c2 * yt1;
    *p->ar = (MYFLT)yt1;
    p->yt1 = yt1;
    return OK;
}

int32_t tone(CSOUND *csound, TONE *p)
{
    IGN(csound);
    MYFLT       *ar, *asig;
    uint32_t offset = p->h.insdshead->ksmps_offset;
    uint32_t early  = p->h.insdshead->ksmps_no_end;
    uint32_t n, nsmps = CS_KSMPS;
    double      c1 = p->c1, c2 = p->c2;
    double      yt1 = p->yt1;

    if (*p->khp != (MYFLT)p->prvhp) {
      double b;
      p->prvhp = (double)*p->khp;
      b = 2.0 - cos((double)(p->prvhp * csound->tpidsr));
      p->c2 = c2 = b - sqrt(b * b - 1.0);
      p->c1 = c1 = 1.0 - c2;
    }
    ar = p->ar;
    asig = p->asig;
    if (UNLIKELY(offset)) memset(ar, '\0', offset*sizeof(MYFLT));
    if (UNLIKELY(early)) {
      nsmps -= early;
      memset(&ar[nsmps], '\0', early*sizeof(MYFLT));
    }
    for (n=offset; n<nsmps; n++) {
      yt1 = c1 * (double)(asig[n]) + c2 * yt1;
      ar[n] = (MYFLT)yt1;
    }
    p->yt1 = yt1;
    return OK;
}

/* TONEX - cascaded first-order filters */

int32_t tonsetx(CSOUND *csound, TONEX *p)
{
    {
      double b;
      p->prvhp = *p->khp;
      b = 2.0 - cos((double)(*p->khp * csound->tpidsr));
      p->c2 = b - sqrt(b * b - 1.0);
      p->c1 = 1.0 - p->c2;
    }
    if (UNLIKELY((p->loop = (int32_t) (*p->ord + FL(0.5))) < 1)) p->loop = 4;
    if (!*p->istor && (p->aux.auxp == NULL ||
                    (uint32_t)(p->loop*sizeof(double)) > p->aux.size))
        csound->AuxAlloc(csound, (int32_t)(p->loop*sizeof(double)), &p->aux);
    p->yt1 = (double*)p->aux.auxp;
    if (LIKELY(!(*p->istor))) {
    memset(p->yt1, 0, p->loop*sizeof(double));
    }
    return OK;
}

int32_t tonex(CSOUND *csound, TONEX *p)
{
    MYFLT       *ar = p->ar;
    double      c2 = p->c2, *yt1 = p->yt1,c1 = p->c1;
    uint32_t    offset = p->h.insdshead->ksmps_offset;
    uint32_t    early  = p->h.insdshead->ksmps_no_end;
    uint32_t    n, nsmps = CS_KSMPS;
    int32_t     j, lp = p->loop;

    if (*p->khp != p->prvhp) {
      double b;
      p->prvhp = (double)*p->khp;
      b = 2.0 - cos(p->prvhp * (double)csound->tpidsr);
      p->c2 = b - sqrt(b * b - 1.0);
      p->c1 = 1.0 - p->c2;
    }

    memmove(ar,p->asig,sizeof(MYFLT)*nsmps);
    if (UNLIKELY(offset))  memset(ar, '\0', offset*sizeof(MYFLT));
    if (UNLIKELY(early)) {
      nsmps -= early;
      memset(&ar[nsmps], '\0', early*sizeof(MYFLT));
    }
    for (j=0; j< lp; j++) {
      for (n=0; n<nsmps; n++) {
        double x = c1 * ar[n] + c2 * yt1[j];
        yt1[j] = x;
        ar[n] = (MYFLT)x;
      }
    }
    return OK;
}

/* ATONE - first-order highpass filter */

int32_t katone(CSOUND *csound, TONE *p)
{
    IGN(csound);
    double     sig, x;
    double      c2 = p->c2, yt1 = p->yt1;

    if (*p->khp != p->prvhp) {
      double b;
      p->prvhp = *p->khp;
      b = 2.0 - cos((double)(*p->khp * CS_ONEDKR *TWOPI));
      p->c2 = c2 = b - sqrt(b * b - 1.0);
    }
      sig = *p->asig;
      x = yt1 = c2 * (yt1 + sig);
      *p->ar = (MYFLT)x;
      yt1 -= sig;

    p->yt1 = yt1;
    return OK;
}


int32_t atone(CSOUND *csound, TONE *p)
{
    MYFLT       *ar, *asig;
    uint32_t    offset = p->h.insdshead->ksmps_offset;
    uint32_t    early  = p->h.insdshead->ksmps_no_end;
    uint32_t    n, nsmps = CS_KSMPS;
    double      c2 = p->c2, yt1 = p->yt1;

    if (*p->khp != p->prvhp) {
      double b;
      p->prvhp = *p->khp;
      b = 2.0 - cos((double)(*p->khp * csound->tpidsr));
      p->c2 = c2 = b - sqrt(b * b - 1.0);
    }
    ar = p->ar;
    if (UNLIKELY(offset)) memset(ar, '\0', offset*sizeof(MYFLT));
    if (UNLIKELY(early)) {
      nsmps -= early;
      memset(&ar[nsmps], '\0', early*sizeof(MYFLT));
    }
    asig = p->asig;
    for (n=offset; n<nsmps; n++) {
      double sig = (double)asig[n];
      double x = yt1 = c2 * (yt1 + sig);
      ar[n] = (MYFLT)x;
      yt1 -= sig;
    }
    p->yt1 = yt1;
    return OK;
}

int32_t atonex(CSOUND *csound, TONEX *p)
{
    MYFLT       *ar = p->ar;
    double      c2 = p->c2, *yt1 = p->yt1;
    uint32_t    offset = p->h.insdshead->ksmps_offset;
    uint32_t    early  = p->h.insdshead->ksmps_no_end;
    uint32_t    n, nsmps = CS_KSMPS;
    int32_t     j, lp = p->loop;

    if (*p->khp != p->prvhp) {
      double b;
      p->prvhp = *p->khp;
      b = 2.0 - cos((double)(*p->khp * csound->tpidsr));
      p->c2 = b - sqrt(b * b - 1.0);
    }

    memmove(ar,p->asig,sizeof(MYFLT)*nsmps);
    if (UNLIKELY(offset)) memset(ar, '\0', offset*sizeof(MYFLT));
    if (UNLIKELY(early)) {
      nsmps -= early;
      memset(&ar[nsmps], '\0', early*sizeof(MYFLT));
    }
    for (j=1; j<lp; j++) {
      for (n=offset; n<nsmps; n++) {
        double sig = (double)ar[n];
        double x = c2 * (yt1[j] + sig);
        yt1[j] = x - sig;
        ar[n] = (MYFLT)x;
      }
    }
    return OK;
}

/* RESON - second-order resonant filter */

int32_t rsnset(CSOUND *csound, RESON *p)
{
    int32_t scale;
    p->scale = scale = (int32_t)*p->iscl;
    if (UNLIKELY(scale && scale != 1 && scale != 2)) {
      return csound->InitError(csound, Str("illegal reson iscl value, %f"),
                                       *p->iscl);
    }
    p->prvcf = p->prvbw = -100.0;
    if (!(*p->istor))
      p->yt1 = p->yt2 = 0.0;
    p->asigf = IS_ASIG_ARG(p->kcf);
    p->asigw = IS_ASIG_ARG(p->kbw);

    return OK;
}

int32_t krsnset(CSOUND *csound, RESON *p){ return rsnset(csound,p); }

int32_t kreson(CSOUND *csound, RESON *p)
{
    uint32_t flag = 0;
    double      c3p1, c3t4, omc3, c2sqr;
    double      yt0, yt1, yt2, c1 = p->c1, c2 = p->c2, c3 = p->c3;
    IGN(csound);

    if (*p->kcf != (MYFLT)p->prvcf) {
      p->prvcf = (double)*p->kcf;
      p->cosf = cos(p->prvcf * (double)(CS_ONEDKR *TWOPI));
      flag = 1;
    }
    if (*p->kbw != (MYFLT)p->prvbw) {
      p->prvbw = (double)*p->kbw;
      c3 = p->c3 = exp(p->prvbw * (double)(-CS_ONEDKR *TWOPI));
      flag = 1;
    }
    if (flag) {
      c3p1 = c3 + 1.0;
      c3t4 = c3 * 4.0;
      omc3 = 1.0 - c3;
      c2 = p->c2 = c3t4 * p->cosf / c3p1;
      c2sqr = c2 * c2;
      if (p->scale == 1)
        c1 = p->c1 = omc3 * sqrt(1.0 - c2sqr / c3t4);
      else if (p->scale == 2)
        c1 = p->c1 = sqrt((c3p1*c3p1-c2sqr) * omc3/c3p1);
      else c1 = p->c1 = 1.0;
    }

    yt1 = p->yt1; yt2 = p->yt2;
    yt0 = c1 * ((double)*p->asig) + c2 * yt1 - c3 * yt2;
    *p->ar = (MYFLT)yt0;
    p->yt1 = yt0; p->yt2 = yt1;
    return OK;
}


int32_t reson(CSOUND *csound, RESON *p)
{
    uint32_t    offset = p->h.insdshead->ksmps_offset;
    uint32_t    early  = p->h.insdshead->ksmps_no_end;
    uint32_t    flag = 0, n, nsmps = CS_KSMPS;
    MYFLT       *ar, *asig;
    double      c3p1, c3t4, omc3, c2sqr;
    double      yt1, yt2, c1 = p->c1, c2 = p->c2, c3 = p->c3;
    int32_t     asigf = p->asigf;
    int32_t     asigw = p->asigw;

    asig = p->asig;
    ar = p->ar;
    if (UNLIKELY(offset)) memset(ar, '\0', offset*sizeof(MYFLT));
    if (UNLIKELY(early)) {
      nsmps -= early;
      memset(&ar[nsmps], '\0', early*sizeof(MYFLT));
    }
    yt1 = p->yt1; yt2 = p->yt2;
    for (n=offset; n<nsmps; n++) {
      double yt0;
      MYFLT cf = asigf ? p->kcf[n] : *p->kcf;
      MYFLT bw = asigw ? p->kbw[n] : *p->kbw;
      if (cf != (MYFLT)p->prvcf) {
        p->prvcf = (double)cf;
        p->cosf = cos(cf * (double)(csound->tpidsr));
        flag = 1;
      }
      if (bw != (MYFLT)p->prvbw) {
        p->prvbw = (double)bw;
        c3 = p->c3 = exp(bw * (double)(csound->mtpdsr));
        flag = 1;
      }
      if (flag) {
        c3p1 = c3 + 1.0;
        c3t4 = c3 * 4.0;
        omc3 = 1.0 - c3;
        c2 = p->c2 = c3t4 * p->cosf / c3p1;
        c2sqr = c2 * c2;
        if (p->scale == 1)
          c1 = p->c1 = omc3 * sqrt(1.0 - c2sqr / c3t4);
        else if (p->scale == 2)
          c1 = p->c1 = sqrt((c3p1*c3p1-c2sqr) * omc3/c3p1);
        else c1 = p->c1 = 1.0;
        flag = 0;
      }
      yt0 = c1 * ((double)asig[n]) + c2 * yt1 - c3 * yt2;
      ar[n] = (MYFLT)yt0;
      yt2 = yt1;
      yt1 = yt0;
    }
    p->yt1 = yt1; p->yt2 = yt2;
    return OK;
}

/* RESONX - cascaded resonant filters */

int32_t rsnsetx(CSOUND *csound, RESONX *p)
{
    int32_t scale;
    p->scale = scale = (int32_t) *p->iscl;
    if ((p->loop = (int32_t) (*p->ord + FL(0.5))) < 1)
      p->loop = 4;
    if (!*p->istor && (p->aux.auxp == NULL ||
                       (uint32_t)(p->loop*2*sizeof(double)) > p->aux.size))
      csound->AuxAlloc(csound, (int32_t)(p->loop*2*sizeof(double)), &p->aux);
    p->yt1 = (double*)p->aux.auxp; p->yt2 = (double*)p->aux.auxp + p->loop;
    if (UNLIKELY(scale && scale != 1 && scale != 2)) {
      return csound->InitError(csound, Str("illegal reson iscl value, %f"),
                                       *p->iscl);
    }
    p->prvcf = p->prvbw = -100.0;

    if (!(*p->istor)) {
      memset(p->yt1, 0, p->loop*sizeof(double));
      memset(p->yt2, 0, p->loop*sizeof(double));
    }
    return OK;
}

int32_t resonx(CSOUND *csound, RESONX *p)
{
    uint32_t    offset = p->h.insdshead->ksmps_offset;
    uint32_t    early  = p->h.insdshead->ksmps_no_end;
    uint32_t    flag = 0, n, nsmps = CS_KSMPS;
    int32_t     j;
    MYFLT       *ar;
    double      c3p1, c3t4, omc3, c2sqr;
    double      *yt1, *yt2, c1,c2,c3;
    int32_t     asgf = IS_ASIG_ARG(p->kcf);
    int32_t     asgw = IS_ASIG_ARG(p->kbw);

    ar   = p->ar;
    c1   = p->c1;
    c2   = p->c2;
    c3   = p->c3;
    yt1  = p->yt1;
    yt2  = p->yt2;
    memmove(ar,p->asig,sizeof(MYFLT)*nsmps);
    if (UNLIKELY(offset)) memset(ar, '\0', offset*sizeof(MYFLT));
    if (UNLIKELY(early)) {
      nsmps -= early;
      memset(&ar[nsmps], '\0', early*sizeof(MYFLT));
    }
    for (j=0; j< p->loop; j++) {
      for (n=offset; n<nsmps; n++) {
        double x;
        MYFLT cf = asgf ? p->kcf[n] : *p->kcf;
        MYFLT bw = asgw ? p->kbw[n] : *p->kbw;
        if (cf != (MYFLT)p->prvcf) {
          p->prvcf = (double)cf;
          p->cosf = cos(cf * (double)(csound->tpidsr));
          flag = 1;
        }
        if (bw != (MYFLT)p->prvbw) {
          p->prvbw = (double)bw;
          c3 = exp(bw * (double)(csound->mtpdsr));
          flag = 1;
        }
        if (flag) {
          c3p1 = c3 + 1.0;
          c3t4 = c3 * 4.0;
          omc3 = 1.0 - c3;
          c2 = c3t4 * p->cosf / c3p1;
          c2sqr = c2 * c2;
          if (p->scale == 1)
            c1 = omc3 * sqrt(1.0 - (c2sqr / c3t4));
          else if (p->scale == 2)
            c1 = sqrt((c3p1*c3p1-c2sqr) * omc3/c3p1);
          else c1 = 1.0;
          flag =0;
        }
        x = c1 * ((double)ar[n]) + c2 * yt1[j] - c3 * yt2[j];
        yt2[j] = yt1[j];
        ar[n] = (MYFLT)x;
        yt1[j] = x;
      }
    }
    p->c1 = c1; p->c2 = c2; p->c3 = c3;
    return OK;
}

/* ARESON - anti-resonant filter */

int32_t kareson(CSOUND *csound, RESON *p)
{
    uint32_t    flag = 0;
    double      c3p1, c3t4, omc3, c2sqr;
    double      yt1, yt2, c1, c2, c3;
    IGN(csound);

    if (*p->kcf != (MYFLT)p->prvcf) {
      p->prvcf = (double)*p->kcf;
      p->cosf = cos(p->prvcf * (double)(CS_ONEDKR *TWOPI));
      flag = 1;
    }
    if (*p->kbw != (MYFLT)p->prvbw) {
      p->prvbw = (double)*p->kbw;
      p->c3 = exp(p->prvbw * (double)(-CS_ONEDKR *TWOPI));
      flag = 1;
    }
    if (flag) {
      c3p1 = p->c3 + 1.0;
      c3t4 = p->c3 * 4.0;
      omc3 = 1.0 - p->c3;
      p->c2 = c3t4 * p->cosf / c3p1;
      c2sqr = p->c2 * p->c2;
      if (p->scale == 1)
        p->c1 = 1.0 - omc3 * sqrt(1.0 - c2sqr / c3t4);
      else if (p->scale == 2)
        p->c1 = 2.0 - sqrt((c3p1*c3p1-c2sqr)*omc3/c3p1);
      else p->c1 = 0.0;
    }

    c1 = p->c1; c2 = p->c2; c3 = p->c3; yt1 = p->yt1; yt2 = p->yt2;
    if (p->scale == 1 || p->scale == 0) {
        double sig = (double) *p->asig;
        double ans = c1 * sig + c2 * yt1 - c3 * yt2;
        yt2 = yt1;
        yt1 = ans - sig;
        *p->ar = (MYFLT)ans;
    }
    else if (p->scale == 2) {
        double sig = (double) *p->asig;
        double ans = c1 * sig + c2 * yt1 - c3 * yt2;
        yt2 = yt1;
        yt1 = ans - 2.0 * sig;
        *p->ar = (MYFLT)ans;
    }
    p->yt1 = yt1; p->yt2 = yt2;
    return OK;
}

int32_t areson(CSOUND *csound, RESON *p)
{
    uint32_t    offset = p->h.insdshead->ksmps_offset;
    uint32_t    early  = p->h.insdshead->ksmps_no_end;
    uint32_t    flag = 0, n, nsmps = CS_KSMPS;
    MYFLT       *ar, *asig;
    double      c3p1, c3t4, omc3, c2sqr;
    double      yt1, yt2, c1, c2, c3;

    if (*p->kcf != (MYFLT)p->prvcf) {
      p->prvcf = (double)*p->kcf;
      p->cosf = cos(p->prvcf * (double)(csound->tpidsr));
      flag = 1;
    }
    if (*p->kbw != (MYFLT)p->prvbw) {
      p->prvbw = (double)*p->kbw;
      p->c3 = exp(p->prvbw * (double)(csound->mtpdsr));
      flag = 1;
    }
    if (flag) {
      c3p1 = p->c3 + 1.0;
      c3t4 = p->c3 * 4.0;
      omc3 = 1.0 - p->c3;
      p->c2 = c3t4 * p->cosf / c3p1;
      c2sqr = p->c2 * p->c2;
      if (p->scale == 1)
        p->c1 = 1.0 - omc3 * sqrt(1.0 - c2sqr / c3t4);
      else if (p->scale == 2)
        p->c1 = 2.0 - sqrt((c3p1*c3p1-c2sqr)*omc3/c3p1);
      else p->c1 = 0.0;
    }
    asig = p->asig;
    ar = p->ar;
    if (UNLIKELY(offset)) memset(ar, '\0', offset*sizeof(MYFLT));
    if (UNLIKELY(early)) {
      nsmps -= early;
      memset(&ar[nsmps], '\0', early*sizeof(MYFLT));
    }
    c1 = p->c1; c2 = p->c2; c3 = p->c3; yt1 = p->yt1; yt2 = p->yt2;
    if (p->scale == 1 || p->scale == 0) {
      for (n=offset; n<nsmps; n++) {
        double sig = (double)asig[n];
        double ans = c1 * sig + c2 * yt1 - c3 * yt2;
        yt2 = yt1;
        yt1 = ans - sig;
        ar[n] = (MYFLT)ans;
      }
    }
    else if (p->scale == 2) {
      for (n=offset; n<nsmps; n++) {
        double sig = (double)asig[n];
        double ans = c1 * sig + c2 * yt1 - c3 * yt2;
        yt2 = yt1;
        yt1 = ans - 2.0 * sig;
        ar[n] = (MYFLT)ans;
      }
    }
    p->yt1 = yt1; p->yt2 = yt2;
    return OK;
}

/* RMS - RMS measurement */

int32_t rmsset(CSOUND *csound, RMS *p)
{
    double   b;

    b = 2.0 - cos((double)(*p->ihp * csound->tpidsr));
    p->c2 = b - sqrt(b*b - 1.0);
    p->c1 = 1.0 - p->c2;
    if (!*p->istor)
      p->prvq = 0.0;
    return OK;
}

int32_t rms(CSOUND *csound, RMS *p)
{
    IGN(csound);
    uint32_t offset = p->h.insdshead->ksmps_offset;
    uint32_t early  = p->h.insdshead->ksmps_no_end;
    uint32_t n, nsmps = CS_KSMPS;
    MYFLT    *asig;
    double   q;
    double   c1 = p->c1, c2 = p->c2;

    q = p->prvq;
    asig = p->asig;
    if (UNLIKELY(early)) nsmps -= early;
    for (n=offset; n<nsmps; n++) {
      double as = (double)asig[n];
      q = c1 * as * as + c2 * q;
    }
    p->prvq = q;
    *p->kr = (MYFLT) sqrt(q);
    return OK;
}

/* GAIN - automatic gain control */

int32_t gainset(CSOUND *csound, GAIN *p)
{
    double   b;

    b = 2.0 - cos((double)(*p->ihp * csound->tpidsr));
    p->c2 = b - sqrt(b*b - 1.0);
    p->c1 = 1.0 - p->c2;
    if (!*p->istor)
      p->prvq = p->prva = 0.0;
    return OK;
}

int32_t gain(CSOUND *csound, GAIN *p)
{
    IGN(csound);
    uint32_t offset = p->h.insdshead->ksmps_offset;
    uint32_t early  = p->h.insdshead->ksmps_no_end;
    uint32_t n, nsmps = CS_KSMPS;
    MYFLT    *ar, *asig;
    double   q, a, m, diff, inc;
    double   c1 = p->c1, c2 = p->c2;

    q = p->prvq;
    asig = p->asig;
    if (UNLIKELY(early)) nsmps -= early;
    for (n = offset; n < nsmps-early; n++) {
      double as = (double)asig[n];
      q = c1 * as * as + c2 * q;
    }
    p->prvq = q;
    if (q > 0.0)
      a = *p->krms / sqrt(q);
    else
      a = *p->krms;
    ar = p->ar;
    if (UNLIKELY(offset)) memset(ar, '\0', offset*sizeof(MYFLT));
    if (UNLIKELY(early)) {
      nsmps -= early;
      memset(&ar[nsmps], '\0', early*sizeof(MYFLT));
    }
    if ((diff = a - p->prva) != 0.0) {
      m = p->prva;
      inc = diff / (double)(nsmps-offset);
      for (n = offset; n < nsmps; n++) {
        ar[n] = asig[n] * m;
        m += inc;
      }
      p->prva = a;
    }
    else {
      for (n = offset; n < nsmps; n++) {
        ar[n] = asig[n] * a;
      }
    }
    return OK;
}

/* BALANCE - balance two signals by RMS */

int32_t balnset(CSOUND *csound, BALANCE *p)
{
    double   b;

    b = 2.0 - cos((double)(*p->ihp * csound->tpidsr));
    p->c2 = b - sqrt(b*b - 1.0);
    p->c1 = 1.0 - p->c2;
    if (!*p->istor)
      p->prvq = p->prvr = p->prva = 0.0;
    return OK;
}

int32_t balance(CSOUND *csound, BALANCE *p)
{
    IGN(csound);
    uint32_t offset = p->h.insdshead->ksmps_offset;
    uint32_t early  = p->h.insdshead->ksmps_no_end;
    uint32_t n, nsmps = CS_KSMPS;
    MYFLT    *ar, *asig, *csig;
    double   q, r, a, m, diff, inc;
    double   c1 = p->c1, c2 = p->c2;

    q = p->prvq;
    r = p->prvr;
    asig = p->asig;
    csig = p->csig;
    ar = p->ar;
    if (UNLIKELY(offset)) memset(ar, '\0', offset*sizeof(MYFLT));
    if (UNLIKELY(early)) {
      nsmps -= early;
      memset(&ar[nsmps], '\0', early*sizeof(MYFLT));
    }
    for (n = offset; n < nsmps; n++) {
      double as = (double)asig[n];
      double cs = (double)csig[n];
      q = c1 * as * as + c2 * q;
      r = c1 * cs * cs + c2 * r;
    }
    p->prvq = q;
    p->prvr = r;
    if (LIKELY(q != 0.0))
      a = sqrt(r/q);
    else
      a = sqrt(r);
    if ((diff = a - p->prva) != 0.0) {
      m = p->prva;
      inc = diff / (double)(nsmps-offset);
      for (n = offset; n < nsmps; n++) {
        ar[n] = asig[n] * m;
        m += inc;
      }
      p->prva = a;
    }
    else {
      for (n = offset; n < nsmps; n++) {
        ar[n] = asig[n] * a;
      }
    }
    return OK;
}

int32_t balance2(CSOUND *csound, BALANCE *p)
{
    IGN(csound);
    uint32_t offset = p->h.insdshead->ksmps_offset;
    uint32_t early  = p->h.insdshead->ksmps_no_end;
    uint32_t n, nsmps = CS_KSMPS;
    MYFLT    *ar, *asig, *csig;
    double   q, r, a;
    double   c1 = p->c1, c2 = p->c2;

    q = p->prvq;
    r = p->prvr;
    asig = p->asig;
    csig = p->csig;
    ar = p->ar;
    if (UNLIKELY(offset)) memset(ar, '\0', offset*sizeof(MYFLT));
    if (UNLIKELY(early)) {
      nsmps -= early;
      memset(&ar[nsmps], '\0', early*sizeof(MYFLT));
    }
    for (n = offset; n < nsmps; n++) {
      double as = (double)asig[n];
      double cs = (double)csig[n];
      q = c1 * as * as + c2 * q;
      r = c1 * cs * cs + c2 * r;
      if (LIKELY(q != 0.0))
        a = sqrt(r/q);
      else
        a = sqrt(r);
      ar[n] = asig[n] * a;
    }
    p->prvq = q;
    p->prvr = r;
    return OK;
}

/* LIMIT - signal limiter */

int32_t klimit(CSOUND *csound, LIMIT *p)
{
    IGN(csound);
    MYFLT       sig=*p->sig, min=*p->min, max=*p->max;
    if (LIKELY((sig <= max) && (sig >= min))) {
      *p->ans = sig;
    }
    else {
     if ( min >= max) {
        *p->ans = FL(0.5) * (min + max);
      }
      else {
        if (sig > max)
          *p->ans = max;
        else
          *p->ans = min;
      }
    }
    return OK;
}

int32_t limit(CSOUND *csound, LIMIT *p)
{
    IGN(csound);
    MYFLT       *ans, *asig;
    MYFLT       min=*p->min, max=*p->max, aver;
    uint32_t    offset = p->h.insdshead->ksmps_offset;
    uint32_t    early  = p->h.insdshead->ksmps_no_end;
    uint32_t    n, nsmps = CS_KSMPS;
    ans = p->ans;
    asig  = p->sig;

    if (UNLIKELY(offset)) memset(ans, '\0', offset*sizeof(MYFLT));
    if (UNLIKELY(early)) {
      nsmps -= early;
      memset(&ans[nsmps], '\0', early*sizeof(MYFLT));
    }
    if (min >= max) {
      aver = (min + max) * FL(0.5);
      for (n=offset; n<nsmps; n++) {
        ans[n] = aver;
      }
    }
    else
      for (n=offset; n<nsmps; n++) {
        if ((asig[n] <= max) && (asig[n] >= min)) {
          ans[n] = asig[n];
        }
        else {
          if (asig[n] > max)
            ans[n] = max;
          else
            ans[n] = min;
        }
      }
    return OK;
}
