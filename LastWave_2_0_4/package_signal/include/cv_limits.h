/*
 * cv_limits.h
 *
 *  Copyright 1997 Centre de Recherche Paul Pascal, Bordeaux, France
 *  Written by Nicolas Decoster, April 1997.
 *
 * The author may be reached (Email) at the address
 *     decoster@crpp.u-bordeaux.fr
 * or  decoster@info.enserb.u-bordeaux.fr
 *
 *   $Id: cv_limits.h,v 1.1 1998/07/07 15:58:20 decoster Exp $
 */

#ifndef _LIM_ARRAY_H_
#define _LIM_ARRAY_H_

#ifndef LIMITS_TAB_SIZE
#define LIMITS_TAB_SIZE 18
#endif

#include "cv_int.h"

static int lim_array[2][3][LIMITS_TAB_SIZE][2] = {
  { /* Analytical */
    {  /* Periodic */
/*       2 */   {2, 2},
/*       4 */   {4, 4},
/*       8 */   {8, 8},
/*      16 */   {6, 6},
/*      32 */   {6, 6},
/*      64 */   {4, 4},
/*     128 */   {4, 4},
/*     256 */   {4, 8},
/*     512 */   {4, 8},
/*    1024 */   {4, 8},
/*    2048 */   {5, 8},
/*    4096 */   {6, 256},
/*    8192 */   {6, 512},
/*   16384 */   {7, 2048},
/*   32768 */   {8, 4096},
/*   65536 */   {13, 16384},
/*  131072 */   {47, 65536},
/*  262144 */   {32, 65536}
    }, {  /* Mirror */
/*       2 */   {2, 2},
/*       4 */   {4, 4},
/*       8 */   {8, 8},
/*      16 */   {6, 8},
/*      32 */   {6, 16},
/*      64 */   {4, 32},
/*     128 */   {4, 64},
/*     256 */   {4, 128},
/*     512 */   {4, 256},
/*    1024 */   {4, 512},
/*    2048 */   {5, 1024},
/*    4096 */   {6, 2048},
/*    8192 */   {6, 4096},
/*   16384 */   {7, 8192},
/*   32768 */   {8, 16384},
/*   65536 */   {13, 32768},
/*  131072 */   {47, 65536},
/*  262144 */   {32, 131072}
    }, {  /* Padding */
/*       2 */   {2, 2},
/*       4 */   {4, 4},
/*       8 */   {8, 8},
/*      16 */   {6, 8},
/*      32 */   {6, 16},
/*      64 */   {4, 32},
/*     128 */   {4, 64},
/*     256 */   {4, 128},
/*     512 */   {4, 256},
/*    1024 */   {4, 512},
/*    2048 */   {5, 1024},
/*    4096 */   {6, 2048},
/*    8192 */   {6, 4096},
/*   16384 */   {7, 8192},
/*   32768 */   {8, 16384},
/*   65536 */   {13, 32768},
/*  131072 */   {47, 65536},
/*  262144 */   {32, 131072}
    }
  }, { /* Numerical */
    {  /* Periodic */
/*       2 */   {2, 2},
/*       4 */   {4, 4},
/*       8 */   {8, 8},
/*      16 */   {6, 6},
/*      32 */   {6, 6},
/*      64 */   {4, 4},
/*     128 */   {4, 4},
/*     256 */   {4, 8},
/*     512 */   {4, 8},
/*    1024 */   {4, 8},
/*    2048 */   {5, 8},
/*    4096 */   {6, 256},
/*    8192 */   {6, 512},
/*   16384 */   {7, 2048},
/*   32768 */   {8, 4096},
/*   65536 */   {13, 16384},
/*  131072 */   {47, 65536},
/*  262144 */   {32, 65536}
    }, {  /* Mirror */
/*       2 */   {2, 2},
/*       4 */   {4, 4},
/*       8 */   {8, 8},
/*      16 */   {6, 8},
/*      32 */   {6, 16},
/*      64 */   {4, 32},
/*     128 */   {4, 64},
/*     256 */   {4, 128},
/*     512 */   {4, 256},
/*    1024 */   {4, 512},
/*    2048 */   {5, 1024},
/*    4096 */   {6, 2048},
/*    8192 */   {6, 4096},
/*   16384 */   {7, 8192},
/*   32768 */   {8, 16384},
/*   65536 */   {13, 32768},
/*  131072 */   {47, 65536},
/*  262144 */   {32, 131072}
    }, {  /* Padding */
/*       2 */   {2, 2},
/*       4 */   {4, 4},
/*       8 */   {8, 8},
/*      16 */   {6, 8},
/*      32 */   {6, 16},
/*      64 */   {4, 32},
/*     128 */   {4, 64},
/*     256 */   {4, 128},
/*     512 */   {4, 256},
/*    1024 */   {4, 512},
/*    2048 */   {5, 1024},
/*    4096 */   {6, 2048},
/*    8192 */   {6, 4096},
/*   16384 */   {7, 8192},
/*   32768 */   {8, 16384},
/*   65536 */   {13, 32768},
/*  131072 */   {47, 65536},
/*  262144 */   {32, 131072}
    }
  }
};

#endif /* _LIM_ARRAY_H_ */
