/*..........................................................................*/
/*                                                                          */
/*      L a s t W a v e    P a c k a g e 'compress2d' 2.0                   */
/*                                                                          */
/*      Copyright (C) 1998-2002 Geoff Davis, Emmanuel Bacry, Jerome Fraleu. */
/*                                                                          */
/*      The original program was written in C++ by Geoff Davis.             */
/*      Then it has been translated in C and adapted to LastWave by         */
/*      J. Fraleu and E. Bacry.                                             */
/*                                                                          */
/*      If you are interested in the C++ code please go to                  */
/*          http://www.cs.dartmouth.edu/~gdavis                             */
/*                                                                          */
/*      emails : geoffd@microsoft.com                                       */
/*               fraleu@cmap.polytechnique.fr                               */
/*               lastwave@cmap.polytechnique.fr                             */
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

#ifndef IHISTOGRAM_H
#define IHISTOGRAM_H

/*
 * IHISTOGRAM
 */

typedef struct iHistogram {
    int totalCount;
    int maxCt;
    int nsyms;
    int *count;			/* pos to ct */
    int *treeCount;		/* pos to ctLeft */
    int *symToPos;		/* symbol to position */
    int *posToSym;		/* position to symbol */
    float onelog2;
} IHistogram, *IHISTOGRAM;


typedef void IHistBinFun(void *closure, int *counts);

extern IHISTOGRAM NewIHistogram(int n, int maxct);
extern void DeleteIHistogram(IHISTOGRAM ih);
extern int TotalCountIHistogram(IHISTOGRAM ih);
extern int LeftCountIHistogram(IHISTOGRAM ih, int sym);
extern void InitCounts1IHistogram(IHISTOGRAM ih, int *cnts);
extern void InitCounts2IHistogram(IHISTOGRAM ih, IHistBinFun *f, void *closure);
extern void IncCountIHistogram(IHISTOGRAM ih, int sym);
extern int CountIHistogram(IHISTOGRAM ih, int sym);
extern int SymbolIHistogram(IHISTOGRAM ih, int val);
extern float  EntropyIHistogram(IHISTOGRAM ih, int sym);

#endif
