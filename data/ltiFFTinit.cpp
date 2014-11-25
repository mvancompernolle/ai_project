/*
 * Copyright (C) 2000, 2001, 2002, 2003, 2004, 2005, 2006
 * Lehrstuhl fuer Technische Informatik, RWTH-Aachen, Germany
 *
 * This file is part of the LTI-Computer Vision Library (LTI-Lib)
 *
 * The LTI-Lib is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License (LGPL)
 * as published by the Free Software Foundation; either version 2.1 of
 * the License, or (at your option) any later version.
 *
 * The LTI-Lib is distributed in the hope that it will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with the LTI-Lib; see the file LICENSE.  If
 * not, write to the Free Software Foundation, Inc., 59 Temple Place -
 * Suite 330, Boston, MA 02111-1307, USA.
 */


/*----------------------------------------------------------------
 * project ....: LTI Digital Image/Signal Processing Library
 * file .......: ltiFFTinit.cpp
 * authors ....: Takuya Ooura, Stefan Syberichs
 * organization: LTI, RWTH Aachen
 * comment.....: inverse FFT routines based on modified code by Takuya Ooura
 *               with kind permission ooura@kurims.kyoto-u.ac.jp or
 *               ooura@mmm.t.u-tokyo.ac.jp)
 * creation ...: 27.01.2000
 * revisions ..: $Id: ltiFFTinit.cpp,v 1.7 2006/02/08 11:04:01 ltilib Exp $
 */

#include <cstdlib>
#include <cstdio>
#include "ltiMath.h"
#include "ltiImage.h"
#include "ltiFFTinit.h"

namespace lti {

  namespace fft {

    // ooura fft 1-d adapted to work on lti::vectors
    template <class T>
    void vrdft(int n, int isgn, lti::vector<T>& a, int *ip, T *w) {
      rdft(n,isgn,&a.at(0),ip,w);
    }

    template <class T>
    // ooura fft 1-d as used by 2-dimensional fft routines
    void rdft(int n, int isgn, T *a, int *ip, T *w) {

      int nw, nc;
      T xi;

      nw = ip[0];
      if (n > (nw << 2)) {
          nw = n >> 2;
          makewt(nw, ip, w);
      }
      nc = ip[1];
      if (n > (nc << 2)) {
          nc = n >> 2;
          makect(nc, ip, w + nw);
      }
      if (isgn >= 0) {
          if (n > 4) {
              bitrv2(n, ip + 2, a);
              cftfsub(n, a, w);
              rftfsub(n, a, nc, w + nw);
          } else if (n == 4) {
              cftfsub(n, a, w);
          }
          xi = a[0] - a[1];
          a[0] += a[1];
          a[1] = xi;
      }
      else {
          a[1] = 0.5f * (a[0] - a[1]);
          a[0] -= a[1];
          if (n > 4) {
              rftbsub(n, a, nc, w + nw);
              bitrv2(n, ip + 2, a);
              cftbsub(n, a, w);
          }
          else if (n == 4) {
              cftfsub(n, a, w);
          }
      }
    }

    // ooura fft 2-d
    template <class T>
    void rdft2d(int n1, int n2, int isgn, lti::matrix<T>& a,
                T *t, int *ip, T *w) {

      int n, nw, nc, n1h, i, j, i2;
      T xi;

      T *zeiger;

      n = n1 << 1;
      if (n < n2) {
          n = n2;
      }
      nw = ip[0];
      if (n > (nw << 2)) {
          nw = n >> 2;
          makewt(nw, ip, w);
      }
      nc = ip[1];
      if (n2 > (nc << 2)) {
          nc = n2 >> 2;
          makect(nc, ip, w + nw);
      }
      n1h = n1 >> 1;
      if (isgn < 0) {
          for (i = 1; i <= n1h - 1; i++) {
              j = n1 - i;
              xi = a[i][0] - a[j][0];
              a[i][0] += a[j][0];
              a[j][0] = xi;
              xi = a[j][1] - a[i][1];
              a[i][1] += a[j][1];
              a[j][1] = xi;
          }
          for (j = 0; j <= n2 - 2; j += 2) {
              for (i = 0; i <= n1 - 1; i++) {
                  i2 = i << 1;
                  t[i2] = a[i][j];
                  t[i2 + 1] = a[i][j + 1];
              }
              cdft(n1 << 1, isgn, t, ip, w);
              for (i = 0; i <= n1 - 1; i++) {
                  i2 = i << 1;
                  a[i][j] = t[i2];
                  a[i][j + 1] = t[i2 + 1];
              }
          }
          for (i = 0; i <= n1 - 1; i++) {
	    zeiger = &(a.getRow(i)[0]);
	    rdft(n2, isgn,&(a.getRow(i)[0]), ip, w); //  rdft(n2, isgn, a[i], ip, w); !!!
          }
      } else {
          for (i = 0; i <= n1 - 1; i++) {
	    zeiger = &(a.getRow(i)[0]);
	    rdft(n2, isgn,&(a.getRow(i)[0]), ip, w); //  rdft(n2, isgn, a[i], ip, w) !!!!
          }
          for (j = 0; j <= n2 - 2; j += 2) {
              for (i = 0; i <= n1 - 1; i++) {
                  i2 = i << 1;
                  t[i2] = a[i][j];
                  t[i2 + 1] = a[i][j + 1];
              }
              cdft(n1 << 1, isgn, t, ip, w);
              for (i = 0; i <= n1 - 1; i++) {
                  i2 = i << 1;
                  a[i][j] = t[i2];
                  a[i][j + 1] = t[i2 + 1];
              }
          }
          for (i = 1; i <= n1h - 1; i++) {
              j = n1 - i;
              a[j][0] = 0.5f * (a[i][0] - a[j][0]);
              a[i][0] -= a[j][0];
              a[j][1] = 0.5f * (a[i][1] + a[j][1]);
              a[i][1] -= a[j][1];
          }
      }
    }

    template<class T>
    T* alloc_1d(const int& n1) {
      T* i;

      i =  new T[n1];
      if (i == 0) {
        throw lti::exception("Allocation Failure!\n");
      }
      return i;
    }

    template<class T>
    void free_1d(T* i) {
      delete[] i;
    }

    template<class T>
    T** alloc_2d(const int& n1, const int& n2) {
      T **ii, *i;
      int j;

      typedef T* tpointer;

      ii = new tpointer[n1];
      if (ii == 0) {
        throw lti::exception("Allocation Failure!\n");
      }

      i = new T[n1*n2];
      if (i == 0) {
        throw lti::exception("Allocation Failure!\n");
      }

      ii[0] = i;
      for (j = 1; j < n1; j++) {
          ii[j] = &i[n2 * j];
      }

      return ii;
    }

    template <class T>
    void free_2d(T** ii) {
      delete[] ii[0];
      delete[] ii;
    }

    template <class T>
    void makewt(int nw, int *ip, T *w) {

      int j, nwh;
      T delta, x, y;

      ip[0] = nw;
      ip[1] = 1;
      if (nw > 2) {
        nwh = nw >> 1;
        delta = static_cast<T>(atan(1.0) / nwh);
        w[0] = 1;
        w[1] = 0;
        w[nwh] = cos(delta * nwh);
        w[nwh + 1] = w[nwh];
        if (nwh > 2) {
          for (j = 2; j < nwh; j += 2) {
            const T angle = delta * j;
            sincos(angle, y, x);
            w[j] = x;
            w[j + 1] = y;
            w[nw - j] = y;
            w[nw - j + 1] = x;
          }
          bitrv2(nw, ip + 2, w);
        }
      }
    }

    template <class T>
    void makect(int nc, int *ip, T *c)
    {
      int j, nch;
      T delta;

      ip[1] = nc;
      if (nc > 1) {
          nch = nc >> 1;
          delta = static_cast<T>(atan(1.0) / nch);
          c[0] = cos(delta * nch);
          c[nch] = 0.5f * c[0];
          T sina, cosa;
          for (j = 1; j < nch; j++) {
            const T angle = delta * j;
            sincos(angle, sina, cosa);
              c[j] = 0.5f * cosa;
              c[nc - j] = 0.5f * sina;
          }
      }
    }


    /* -------- child routines -------- */
    template <class T>
    void bitrv2(int n, int *ip, T *a) {
      int j, j1, k, k1, l, m, m2;
      T xr, xi, yr, yi;

      ip[0] = 0;
      l = n;
      m = 1;
      while ((m << 3) < l) {
        l >>= 1;
        for (j = 0; j < m; j++) {
          ip[m + j] = ip[j] + l;
        }
        m <<= 1;
      }
      m2 = 2 * m;
      if ((m << 3) == l) {
        for (k = 0; k < m; k++) {
          for (j = 0; j < k; j++) {
            j1 = 2 * j + ip[k];
            k1 = 2 * k + ip[j];
            xr = a[j1];
            xi = a[j1 + 1];
            yr = a[k1];
            yi = a[k1 + 1];
            a[j1] = yr;
            a[j1 + 1] = yi;
            a[k1] = xr;
            a[k1 + 1] = xi;
            j1 += m2;
            k1 += 2 * m2;
            xr = a[j1];
            xi = a[j1 + 1];
            yr = a[k1];
            yi = a[k1 + 1];
            a[j1] = yr;
            a[j1 + 1] = yi;
            a[k1] = xr;
            a[k1 + 1] = xi;
            j1 += m2;
            k1 -= m2;
            xr = a[j1];
            xi = a[j1 + 1];
            yr = a[k1];
            yi = a[k1 + 1];
            a[j1] = yr;
            a[j1 + 1] = yi;
            a[k1] = xr;
            a[k1 + 1] = xi;
            j1 += m2;
            k1 += 2 * m2;
            xr = a[j1];
            xi = a[j1 + 1];
            yr = a[k1];
            yi = a[k1 + 1];
            a[j1] = yr;
            a[j1 + 1] = yi;
            a[k1] = xr;
            a[k1 + 1] = xi;
          }
          j1 = 2 * k + m2 + ip[k];
          k1 = j1 + m2;
          xr = a[j1];
          xi = a[j1 + 1];
          yr = a[k1];
          yi = a[k1 + 1];
          a[j1] = yr;
          a[j1 + 1] = yi;
          a[k1] = xr;
          a[k1 + 1] = xi;
        }
      } else {
        for (k = 1; k < m; k++) {
          for (j = 0; j < k; j++) {
            j1 = 2 * j + ip[k];
            k1 = 2 * k + ip[j];
            xr = a[j1];
            xi = a[j1 + 1];
            yr = a[k1];
            yi = a[k1 + 1];
            a[j1] = yr;
            a[j1 + 1] = yi;
            a[k1] = xr;
            a[k1 + 1] = xi;
            j1 += m2;
            k1 += m2;
            xr = a[j1];
            xi = a[j1 + 1];
            yr = a[k1];
            yi = a[k1 + 1];
            a[j1] = yr;
            a[j1 + 1] = yi;
            a[k1] = xr;
            a[k1 + 1] = xi;
          }
        }
      }
    }

    template <class T>
    void bitrv2conj(int n, int *ip, T *a)  {
      int j, j1, k, k1, l, m, m2;
      T xr, xi, yr, yi;

      ip[0] = 0;
      l = n;
      m = 1;
      while ((m << 3) < l) {
        l >>= 1;
        for (j = 0; j < m; j++) {
          ip[m + j] = ip[j] + l;
        }
        m <<= 1;
      }
      m2 = 2 * m;
      if ((m << 3) == l) {
        for (k = 0; k < m; k++) {
          for (j = 0; j < k; j++) {
            j1 = 2 * j + ip[k];
            k1 = 2 * k + ip[j];
            xr = a[j1];
            xi = -a[j1 + 1];
            yr = a[k1];
            yi = -a[k1 + 1];
            a[j1] = yr;
            a[j1 + 1] = yi;
            a[k1] = xr;
            a[k1 + 1] = xi;
            j1 += m2;
            k1 += 2 * m2;
            xr = a[j1];
            xi = -a[j1 + 1];
            yr = a[k1];
            yi = -a[k1 + 1];
            a[j1] = yr;
            a[j1 + 1] = yi;
            a[k1] = xr;
            a[k1 + 1] = xi;
            j1 += m2;
            k1 -= m2;
            xr = a[j1];
            xi = -a[j1 + 1];
            yr = a[k1];
            yi = -a[k1 + 1];
            a[j1] = yr;
            a[j1 + 1] = yi;
            a[k1] = xr;
            a[k1 + 1] = xi;
            j1 += m2;
            k1 += 2 * m2;
            xr = a[j1];
            xi = -a[j1 + 1];
            yr = a[k1];
            yi = -a[k1 + 1];
            a[j1] = yr;
            a[j1 + 1] = yi;
            a[k1] = xr;
            a[k1 + 1] = xi;
          }
          k1 = 2 * k + ip[k];
          a[k1 + 1] = -a[k1 + 1];
          j1 = k1 + m2;
          k1 = j1 + m2;
          xr = a[j1];
          xi = -a[j1 + 1];
          yr = a[k1];
          yi = -a[k1 + 1];
          a[j1] = yr;
          a[j1 + 1] = yi;
          a[k1] = xr;
          a[k1 + 1] = xi;
          k1 += m2;
          a[k1 + 1] = -a[k1 + 1];
        }
      } else {
        a[1] = -a[1];
        a[m2 + 1] = -a[m2 + 1];
        for (k = 1; k < m; k++) {
          for (j = 0; j < k; j++) {
            j1 = 2 * j + ip[k];
            k1 = 2 * k + ip[j];
            xr = a[j1];
            xi = -a[j1 + 1];
            yr = a[k1];
            yi = -a[k1 + 1];
            a[j1] = yr;
            a[j1 + 1] = yi;
            a[k1] = xr;
            a[k1 + 1] = xi;
            j1 += m2;
            k1 += m2;
            xr = a[j1];
            xi = -a[j1 + 1];
            yr = a[k1];
            yi = -a[k1 + 1];
            a[j1] = yr;
            a[j1 + 1] = yi;
            a[k1] = xr;
            a[k1 + 1] = xi;
          }
          k1 = 2 * k + ip[k];
          a[k1 + 1] = -a[k1 + 1];
          a[k1 + m2 + 1] = -a[k1 + m2 + 1];
        }
      }
    }

    template <class T>
    void cftfsub(int n, T *a, T *w)
    {
      int j, j1, j2, j3, l;
      T x0r, x0i, x1r, x1i, x2r, x2i, x3r, x3i;

      l = 2;
      if (n > 8) {
          cft1st(n, a, w);
          l = 8;
          while ((l << 2) < n) {
              cftmdl(n, l, a, w);
              l <<= 2;
          }
      }
      if ((l << 2) == n) {
          for (j = 0; j < l; j += 2) {
              j1 = j + l;
              j2 = j1 + l;
              j3 = j2 + l;
              x0r = a[j] + a[j1];
              x0i = a[j + 1] + a[j1 + 1];
              x1r = a[j] - a[j1];
              x1i = a[j + 1] - a[j1 + 1];
              x2r = a[j2] + a[j3];
              x2i = a[j2 + 1] + a[j3 + 1];
              x3r = a[j2] - a[j3];
              x3i = a[j2 + 1] - a[j3 + 1];
              a[j] = x0r + x2r;
              a[j + 1] = x0i + x2i;
              a[j2] = x0r - x2r;
              a[j2 + 1] = x0i - x2i;
              a[j1] = x1r - x3i;
              a[j1 + 1] = x1i + x3r;
              a[j3] = x1r + x3i;
              a[j3 + 1] = x1i - x3r;
          }
      } else {
          for (j = 0; j < l; j += 2) {
              j1 = j + l;
              x0r = a[j] - a[j1];
              x0i = a[j + 1] - a[j1 + 1];
              a[j] += a[j1];
              a[j + 1] += a[j1 + 1];
              a[j1] = x0r;
              a[j1 + 1] = x0i;
          }
      }
    }

    template <class T>
    void cftbsub(int n, T *a, T *w)
    {
      int j, j1, j2, j3, l;
      T x0r, x0i, x1r, x1i, x2r, x2i, x3r, x3i;

      l = 2;
      if (n > 8) {
          cft1st(n, a, w);
          l = 8;
          while ((l << 2) < n) {
              cftmdl(n, l, a, w);
              l <<= 2;
          }
      }
      if ((l << 2) == n) {
          for (j = 0; j < l; j += 2) {
              j1 = j + l;
              j2 = j1 + l;
              j3 = j2 + l;
              x0r = a[j] + a[j1];
              x0i = -a[j + 1] - a[j1 + 1];
              x1r = a[j] - a[j1];
              x1i = -a[j + 1] + a[j1 + 1];
              x2r = a[j2] + a[j3];
              x2i = a[j2 + 1] + a[j3 + 1];
              x3r = a[j2] - a[j3];
              x3i = a[j2 + 1] - a[j3 + 1];
              a[j] = x0r + x2r;
              a[j + 1] = x0i - x2i;
              a[j2] = x0r - x2r;
              a[j2 + 1] = x0i + x2i;
              a[j1] = x1r - x3i;
              a[j1 + 1] = x1i - x3r;
              a[j3] = x1r + x3i;
              a[j3 + 1] = x1i + x3r;
          }
      } else {
          for (j = 0; j < l; j += 2) {
              j1 = j + l;
              x0r = a[j] - a[j1];
              x0i = -a[j + 1] + a[j1 + 1];
              a[j] += a[j1];
              a[j + 1] = -a[j + 1] - a[j1 + 1];
              a[j1] = x0r;
              a[j1 + 1] = x0i;
          }
      }
    }

    template <class T>
    void cft1st(int n, T *a, T *w)
    {
      int j, k1, k2;
      T wk1r, wk1i, wk2r, wk2i, wk3r, wk3i;
      T x0r, x0i, x1r, x1i, x2r, x2i, x3r, x3i;

      x0r = a[0] + a[2];
      x0i = a[1] + a[3];
      x1r = a[0] - a[2];
      x1i = a[1] - a[3];
      x2r = a[4] + a[6];
      x2i = a[5] + a[7];
      x3r = a[4] - a[6];
      x3i = a[5] - a[7];
      a[0] = x0r + x2r;
      a[1] = x0i + x2i;
      a[4] = x0r - x2r;
      a[5] = x0i - x2i;
      a[2] = x1r - x3i;
      a[3] = x1i + x3r;
      a[6] = x1r + x3i;
      a[7] = x1i - x3r;
      wk1r = w[2];
      x0r = a[8] + a[10];
      x0i = a[9] + a[11];
      x1r = a[8] - a[10];
      x1i = a[9] - a[11];
      x2r = a[12] + a[14];
      x2i = a[13] + a[15];
      x3r = a[12] - a[14];
      x3i = a[13] - a[15];
      a[8] = x0r + x2r;
      a[9] = x0i + x2i;
      a[12] = x2i - x0i;
      a[13] = x0r - x2r;
      x0r = x1r - x3i;
      x0i = x1i + x3r;
      a[10] = wk1r * (x0r - x0i);
      a[11] = wk1r * (x0r + x0i);
      x0r = x3i + x1r;
      x0i = x3r - x1i;
      a[14] = wk1r * (x0i - x0r);
      a[15] = wk1r * (x0i + x0r);
      k1 = 0;
      for (j = 16; j < n; j += 16) {
          k1 += 2;
          k2 = 2 * k1;
          wk2r = w[k1];
          wk2i = w[k1 + 1];
          wk1r = w[k2];
          wk1i = w[k2 + 1];
          wk3r = wk1r - 2 * wk2i * wk1i;
          wk3i = 2 * wk2i * wk1r - wk1i;
          x0r = a[j] + a[j + 2];
          x0i = a[j + 1] + a[j + 3];
          x1r = a[j] - a[j + 2];
          x1i = a[j + 1] - a[j + 3];
          x2r = a[j + 4] + a[j + 6];
          x2i = a[j + 5] + a[j + 7];
          x3r = a[j + 4] - a[j + 6];
          x3i = a[j + 5] - a[j + 7];
          a[j] = x0r + x2r;
          a[j + 1] = x0i + x2i;
          x0r -= x2r;
          x0i -= x2i;
          a[j + 4] = wk2r * x0r - wk2i * x0i;
          a[j + 5] = wk2r * x0i + wk2i * x0r;
          x0r = x1r - x3i;
          x0i = x1i + x3r;
          a[j + 2] = wk1r * x0r - wk1i * x0i;
          a[j + 3] = wk1r * x0i + wk1i * x0r;
          x0r = x1r + x3i;
          x0i = x1i - x3r;
          a[j + 6] = wk3r * x0r - wk3i * x0i;
          a[j + 7] = wk3r * x0i + wk3i * x0r;
          wk1r = w[k2 + 2];
          wk1i = w[k2 + 3];
          wk3r = wk1r - 2 * wk2r * wk1i;
          wk3i = 2 * wk2r * wk1r - wk1i;
          x0r = a[j + 8] + a[j + 10];
          x0i = a[j + 9] + a[j + 11];
          x1r = a[j + 8] - a[j + 10];
          x1i = a[j + 9] - a[j + 11];
          x2r = a[j + 12] + a[j + 14];
          x2i = a[j + 13] + a[j + 15];
          x3r = a[j + 12] - a[j + 14];
          x3i = a[j + 13] - a[j + 15];
          a[j + 8] = x0r + x2r;
          a[j + 9] = x0i + x2i;
          x0r -= x2r;
          x0i -= x2i;
          a[j + 12] = -wk2i * x0r - wk2r * x0i;
          a[j + 13] = -wk2i * x0i + wk2r * x0r;
          x0r = x1r - x3i;
          x0i = x1i + x3r;
          a[j + 10] = wk1r * x0r - wk1i * x0i;
          a[j + 11] = wk1r * x0i + wk1i * x0r;
          x0r = x1r + x3i;
          x0i = x1i - x3r;
          a[j + 14] = wk3r * x0r - wk3i * x0i;
          a[j + 15] = wk3r * x0i + wk3i * x0r;
      }
    }


    template <class T>
    void cftmdl(int n, int l, T *a, T *w)
    {
      int j, j1, j2, j3, k, k1, k2, m, m2;
      T wk1r, wk1i, wk2r, wk2i, wk3r, wk3i;
      T x0r, x0i, x1r, x1i, x2r, x2i, x3r, x3i;

      m = l << 2;
      for (j = 0; j < l; j += 2) {
          j1 = j + l;
          j2 = j1 + l;
          j3 = j2 + l;
          x0r = a[j] + a[j1];
          x0i = a[j + 1] + a[j1 + 1];
          x1r = a[j] - a[j1];
          x1i = a[j + 1] - a[j1 + 1];
          x2r = a[j2] + a[j3];
          x2i = a[j2 + 1] + a[j3 + 1];
          x3r = a[j2] - a[j3];
          x3i = a[j2 + 1] - a[j3 + 1];
          a[j] = x0r + x2r;
          a[j + 1] = x0i + x2i;
          a[j2] = x0r - x2r;
          a[j2 + 1] = x0i - x2i;
          a[j1] = x1r - x3i;
          a[j1 + 1] = x1i + x3r;
          a[j3] = x1r + x3i;
          a[j3 + 1] = x1i - x3r;
      }
      wk1r = w[2];
      for (j = m; j < l + m; j += 2) {
          j1 = j + l;
          j2 = j1 + l;
          j3 = j2 + l;
          x0r = a[j] + a[j1];
          x0i = a[j + 1] + a[j1 + 1];
          x1r = a[j] - a[j1];
          x1i = a[j + 1] - a[j1 + 1];
          x2r = a[j2] + a[j3];
          x2i = a[j2 + 1] + a[j3 + 1];
          x3r = a[j2] - a[j3];
          x3i = a[j2 + 1] - a[j3 + 1];
          a[j] = x0r + x2r;
          a[j + 1] = x0i + x2i;
          a[j2] = x2i - x0i;
          a[j2 + 1] = x0r - x2r;
          x0r = x1r - x3i;
          x0i = x1i + x3r;
          a[j1] = wk1r * (x0r - x0i);
          a[j1 + 1] = wk1r * (x0r + x0i);
          x0r = x3i + x1r;
          x0i = x3r - x1i;
          a[j3] = wk1r * (x0i - x0r);
          a[j3 + 1] = wk1r * (x0i + x0r);
      }
      k1 = 0;
      m2 = 2 * m;
      for (k = m2; k < n; k += m2) {
          k1 += 2;
          k2 = 2 * k1;
          wk2r = w[k1];
          wk2i = w[k1 + 1];
          wk1r = w[k2];
          wk1i = w[k2 + 1];
          wk3r = wk1r - 2 * wk2i * wk1i;
          wk3i = 2 * wk2i * wk1r - wk1i;
          for (j = k; j < l + k; j += 2) {
              j1 = j + l;
              j2 = j1 + l;
              j3 = j2 + l;
              x0r = a[j] + a[j1];
              x0i = a[j + 1] + a[j1 + 1];
              x1r = a[j] - a[j1];
              x1i = a[j + 1] - a[j1 + 1];
              x2r = a[j2] + a[j3];
              x2i = a[j2 + 1] + a[j3 + 1];
              x3r = a[j2] - a[j3];
              x3i = a[j2 + 1] - a[j3 + 1];
              a[j] = x0r + x2r;
              a[j + 1] = x0i + x2i;
              x0r -= x2r;
              x0i -= x2i;
              a[j2] = wk2r * x0r - wk2i * x0i;
              a[j2 + 1] = wk2r * x0i + wk2i * x0r;
              x0r = x1r - x3i;
              x0i = x1i + x3r;
              a[j1] = wk1r * x0r - wk1i * x0i;
              a[j1 + 1] = wk1r * x0i + wk1i * x0r;
              x0r = x1r + x3i;
              x0i = x1i - x3r;
              a[j3] = wk3r * x0r - wk3i * x0i;
              a[j3 + 1] = wk3r * x0i + wk3i * x0r;
          }
          wk1r = w[k2 + 2];
          wk1i = w[k2 + 3];
          wk3r = wk1r - 2 * wk2r * wk1i;
          wk3i = 2 * wk2r * wk1r - wk1i;
          for (j = k + m; j < l + (k + m); j += 2) {
              j1 = j + l;
              j2 = j1 + l;
              j3 = j2 + l;
              x0r = a[j] + a[j1];
              x0i = a[j + 1] + a[j1 + 1];
              x1r = a[j] - a[j1];
              x1i = a[j + 1] - a[j1 + 1];
              x2r = a[j2] + a[j3];
              x2i = a[j2 + 1] + a[j3 + 1];
              x3r = a[j2] - a[j3];
              x3i = a[j2 + 1] - a[j3 + 1];
              a[j] = x0r + x2r;
              a[j + 1] = x0i + x2i;
              x0r -= x2r;
              x0i -= x2i;
              a[j2] = -wk2i * x0r - wk2r * x0i;
              a[j2 + 1] = -wk2i * x0i + wk2r * x0r;
              x0r = x1r - x3i;
              x0i = x1i + x3r;
              a[j1] = wk1r * x0r - wk1i * x0i;
              a[j1 + 1] = wk1r * x0i + wk1i * x0r;
              x0r = x1r + x3i;
              x0i = x1i - x3r;
              a[j3] = wk3r * x0r - wk3i * x0i;
              a[j3 + 1] = wk3r * x0i + wk3i * x0r;
          }
      }
    }


    template <class T>
    void rftfsub(int n, T *a, int nc, T *c)
    {
      int j, k, kk, ks, m;
      T wkr, wki, xr, xi, yr, yi;

      m = n >> 1;
      ks = 2 * nc / m;
      kk = 0;
      for (j = 2; j < m; j += 2) {
          k = n - j;
          kk += ks;
          wkr = 0.5f - c[nc - kk];
          wki = c[kk];
          xr = a[j] - a[k];
          xi = a[j + 1] + a[k + 1];
          yr = wkr * xr - wki * xi;
          yi = wkr * xi + wki * xr;
          a[j] -= yr;
          a[j + 1] -= yi;
          a[k] += yr;
          a[k + 1] -= yi;
      }
    }


    template <class T>
    void rftbsub(int n, T *a, int nc, T *c)
    {
      int j, k, kk, ks, m;
      T wkr, wki, xr, xi, yr, yi;

      a[1] = -a[1];
      m = n >> 1;
      ks = 2 * nc / m;
      kk = 0;
      for (j = 2; j < m; j += 2) {
          k = n - j;
          kk += ks;
          wkr = 0.5f - c[nc - kk];
          wki = c[kk];
          xr = a[j] - a[k];
          xi = a[j + 1] + a[k + 1];
          yr = wkr * xr + wki * xi;
          yi = wkr * xi - wki * xr;
          a[j] -= yr;
          a[j + 1] = yi - a[j + 1];
          a[k] += yr;
          a[k + 1] = yi - a[k + 1];
      }
      a[m + 1] = -a[m + 1];
    }


    template <class T>
    void dctsub(int n, T *a, int nc, T *c)
    {
      int j, k, kk, ks, m;
      T wkr, wki, xr;

      m = n >> 1;
      ks = nc / n;
      kk = 0;
      for (j = 1; j < m; j++) {
          k = n - j;
          kk += ks;
          wkr = c[kk] - c[nc - kk];
          wki = c[kk] + c[nc - kk];
          xr = wki * a[j] - wkr * a[k];
          a[j] = wkr * a[j] + wki * a[k];
          a[k] = xr;
      }
      a[m] *= c[0];
    }


    template <class T>
    void dstsub(int n, T *a, int nc, T *c)
    {
      int j, k, kk, ks, m;
      T wkr, wki, xr;

      m = n >> 1;
      ks = nc / n;
      kk = 0;
      for (j = 1; j < m; j++) {
          k = n - j;
          kk += ks;
          wkr = c[kk] - c[nc - kk];
          wki = c[kk] + c[nc - kk];
          xr = wki * a[k] - wkr * a[j];
          a[k] = wkr * a[k] + wki * a[j];
          a[j] = xr;
      }
      a[m] *= c[0];
    }


    template <class T>
    void cdft(int n, int isgn, T *a, int *ip, T *w)
    {
      if (n > (ip[0] << 2)) {
          makewt(n >> 2, ip, w);
      }
      if (n > 4) {
          if (isgn >= 0) {
              bitrv2(n, ip + 2, a);
              cftfsub(n, a, w);
          } else {
              bitrv2conj(n, ip + 2, a);
              cftbsub(n, a, w);
          }
      } else if (n == 4) {
          cftfsub(n, a, w);
      }
    }

    // explicit instatiation
    template int* alloc_1d<int>(const int& n1);
    template void free_1d<int>(int* i);
    template float* alloc_1d<float>(const int& n1);
    template void free_1d<float>(float* i);
    template double* alloc_1d<double>(const int& n1);
    template void free_1d<double>(double* i);

    template int** alloc_2d<int>(const int& n1,const int& n2);
    template void free_2d<int>(int** i);
    template float** alloc_2d<float>(const int& n1,const int& n2);
    template void free_2d<float>(float** i);
    template double** alloc_2d<double>(const int& n1,const int& n2);
    template void free_2d<double>(double** i);

    template void vrdft<float>(int n, int isgn, vector<float>& a, int *ip,
                               float *w);
    template void vrdft<double>(int n, int isgn, vector<double>& a, int *ip,
                                double *w);

    template void rdft2d<float>(int, int, int, matrix<float>&,
                                float*, int*, float*);
    template void rdft2d<double>(int, int, int, matrix<double>&,
                                 double*, int*, double*);

  } // end namespace fft
} // end namespace lti
