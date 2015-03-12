/*..........................................................................*/
/*                                                                          */
/*      L a s t W a v e   K e r n e l   2 . 0                               */
/*                                                                          */
/*      Copyright (C) 1998-2002 Emmanuel Bacry.                             */
/*      email : lastwave@cmap.polytechnique.fr                              */
/*                                                                          */
/*..........................................................................*/
/*                                                                          */
/*      This program is a free software, you can redistribute it and/or     */
/*      modify it under the terms of the GNU General Public License as      */
/*      published by the Free Software Foundation; either version 2 of the  */
/*      License, or (at your option) any later version                      */
/*                                                                          */
/*      This program is distributed in the hope that it will be useful,     */
/*      but WITHOUT ANY WARRANTY; without even the implied warranty of      */
/*      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the       */
/*      GNU General Public License for more details.                        */
/*                                                                          */
/*      You should have received a copy of the GNU General Public License   */
/*      along with this program (in a file named COPYRIGHT);                */
/*      if not, write to the Free Software Foundation, Inc.,                */
/*      59 Temple Place, Suite 330, Boston, MA  02111-1307  USA             */
/*                                                                          */
/*..........................................................................*/


/********************************************************************/
/*                                                                  */
/*   lastwave.h :                  main include                       */
/*                                                                  */
/********************************************************************/

#ifndef _LASTWAVE_H
#define _LASTWAVE_H

#ifdef __cplusplus
extern "C" {
#endif


#include <stdio.h>
#include <setjmp.h>

#include <math.h>
#include <string.h>
#include <float.h>
#include <stdlib.h>

#include <limits.h>
#include <ctype.h>

#include "computer.h"


#define NO  0	/* Some useful constants */
#define YES 1
#define ERROR (-1)

#define STRSIZE 200
#define STRSIZ  20


#ifndef M_PI 
#define M_PI 3.14159265358979323846
#endif

/* Max of two numbers */
#ifndef MAX
#define MAX(x,y)  ((x) > (y) ? (x) : (y))
#endif

/* Min of two numbers */
#ifndef MIN
#define MIN(x,y)  ((x) < (y) ? (x) : (y))
#endif

/* Sign of a number */
#define SIGN(x)  ((x) < 0 ? (-1) : (x) > 0 ? 1 : 0)

/* returns YES iff inf <= x <= sup */
#define INRANGE(inf,x,sup)  ((inf) <= (x) && (x) <= (sup)) 

/* Checking the different domains for the mathematical functions */
#define MyLog(f) ((f) <= 0 ? Warningf("Bad value '%g' for ln function",(f)), -FLT_MAX/2 : log((f)))
#define MyLog10(f) ((f) <= 0 ? Warningf("Bad value '%g' for log function",(f)), -FLT_MAX/2 : log10((f)))
#define MySqrt(f) ((f) < 0 ? Warningf("Bad value '%g' for sqrt function",(f)), 0. : sqrt((f)))
#define MyLog2(f) ((f) <= 0 ? Warningf("Bad value '%g' for log2 function",(f)), -FLT_MAX/2 : log((f))/.69314718055995)

/* In misc.c */
extern void QuickSort(double *x,int n);
extern float Urand(void);
extern void RandInit(long int init);
extern float Grand(float sigma);
extern float MyTime(void);


/* 
 * Macros 'IsCPULittleEndian', 'IsCPUBigEndian' to know wether the CPU is little endian or big endian.
 * They use the variable 'cpuBinaryMode' which is set at startup  by the procedure 'InitCPUBinaryMode'
 * in int_misc.c
 */
extern char cpuBinaryMode;

enum { 
  BinaryLittleEndian = 1,
  BinaryBigEndian
};

#define IsCPULittleEndian (cpuBinaryMode == BinaryLittleEndian)
#define IsCPUBigEndian (cpuBinaryMode == BinaryBigEndian)

/* 
 * Function to convert Big (resp. Little) values to Little (resp. Big) values.
 * Defined in 'int_misc.c'
 */
extern void BigLittleValues(void *array, int n, size_t sizeval);


/**********************/
/* Include some files */
/**********************/

#include "int_value.h"
#include "int_hash.h"
#include "int_streams.h"
#include "int_procs.h"
#include "int_history.h"
#include "gobject.h"
#include "int_toplevel.h"
#include "int.h"
											
#include "int_variables.h"
#include "int_parser.h"

#include "terminal.h"		


/* A flag for debugging allocations */
/* #define DEBUGALLOC 1   */


#ifdef DEBUGALLOC
extern char *DebugType;
#endif

#ifdef __cplusplus
}
#endif

#endif
