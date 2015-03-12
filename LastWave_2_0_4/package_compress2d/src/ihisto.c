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


#include "lastwave.h"
#include "ihisto.h"



/*
 * IHISTOGRAM 
 */

IHISTOGRAM NewIHistogram(int n, int maxct)
{
  IHISTOGRAM ih;
  int i;
  
  ih = (IHISTOGRAM) Malloc(sizeof(IHistogram));

  ih->onelog2 = 1/log(2.0);
  ih->nsyms = n;
  ih->maxCt = maxct;
  ih->totalCount = 0;
  ih->count = IntAlloc(ih->nsyms);
  ih->treeCount = IntAlloc(ih->nsyms);
  ih->symToPos = IntAlloc(ih->nsyms);
  ih->posToSym = IntAlloc(ih->nsyms);

  for (i = 0; i < ih->nsyms; i++) {
	ih->count[i] = 0;
	ih->treeCount[i] = 0;
	ih->symToPos[i] = i;
	ih->posToSym[i] = i;
  }    
  
  return(ih);
}

void DeleteIHistogram(IHISTOGRAM ih)
{
  Free(ih->count);
  Free(ih->treeCount);
  Free(ih->symToPos);
  Free(ih->posToSym);
  Free(ih);
}

static int Parent(int child) { return (child - 1) >> 1; }
static int Lchild(int parent) { return parent + parent + 1; }
static int Rchild(int parent) { return parent + parent + 2; }

int TotalCountIHistogram(IHISTOGRAM ih)
{
  return(ih->totalCount);
}

int LeftCountIHistogram(IHISTOGRAM ih, int sym)
{
    int lc = 0;
    int pos = ih->symToPos[sym];
    int parent;
    
    lc = ih->treeCount[pos];

    while (pos) {
	  parent = Parent(pos);
	  if (Rchild(parent) == pos) lc += ih->treeCount[parent] + ih->count[parent];
	  pos = parent;
    }
    return (lc);
}

int CountIHistogram(IHISTOGRAM ih, int sym) 
{
  return ih->count[ih->symToPos[sym]];
}


static void BuildTreeCountIHistogram(IHISTOGRAM ih)
{
    int i;
    int pos;
    int parent;
    
    for (i = 0; i < ih->nsyms; i++) { ih->treeCount[i] = 0; }

    for (i = ih->nsyms - 1; i > 0; i--) {
	pos = i;
	while (pos > 0) {
	    parent = Parent(pos);
	    if (pos == Lchild(parent)) {
		ih->treeCount[parent] += ih->count[i];
	    }
	    pos = parent;
	}
    }
}

static void ScaleCountsIHistogram(IHISTOGRAM ih)
{
 int i;
 
    ih->totalCount = 0;
    for (i = 0; i < ih->nsyms; i++) {
	if (ih->count[i] == 0) {
	    ;			/* skip this one */
	} else {
	    ih->count[i] = ih->count[i] / 2;
	    if (!ih->count[i]) {
		ih->count[i] = 1;
	    }
	    ih->totalCount += ih->count[i];
	}
    }
}

static void ReorganiseIHistogram(IHISTOGRAM ih,int sym)
{
    int pos = ih->symToPos[sym];
  int right, left,middle;
  int rsym;
  
    if (!pos || ih->count[pos - 1] > ih->count[pos]) {
	return;
    }

    /* find leftmost count == count[pos], then swap values */

    right = pos;
    left  = 0;

    /* make sure it isn't under left at start. We'll never put it 
     there later. */
    if (ih->count[left] == ih->count[pos]) { right = left; }

    while (left != right) {
	middle = (left + right) / 2;
	if (ih->count[middle] > ih->count[pos]) {
	    /*middle is to left of leftmost count == count[pos] */
	    left = middle + 1;
	} else {
	    /*leftmost count == count[pos] is either under middle, or to left
	    of middle. */
	    right = middle;
	}
    }

    rsym = ih->posToSym[right];

    ih->symToPos[sym] = right;
    ih->posToSym[pos] = ih->posToSym[right];
    ih->symToPos[rsym] = pos;
    ih->posToSym[right] = sym;
}

static void SwapSyms(IHISTOGRAM ih,int pos_a, int pos_b)
{
    int sym_a = ih->posToSym[pos_a];
    int sym_b = ih->posToSym[pos_b];
    int t;
    
    ih->symToPos[sym_a] = pos_b;
    ih->symToPos[sym_b] = pos_a;
    
    ih->posToSym[pos_a] = sym_b;
    ih->posToSym[pos_b] = sym_a;
    
    t = ih->count[pos_a];
    ih->count[pos_a] = ih->count[pos_b];
    ih->count[pos_b] = t;
}


static int Sorted(IHISTOGRAM ih, int from, int to)
{
  int i;
  
    for (i = from; i < to - 1; i++) {
	  if (ih->count[i] < ih->count[i + 1]) return 0;
    }
    return 1;
}

static void Qs(IHISTOGRAM ih, int from, int to)
{
    int pivot = from;
    int ub = to;
       
    if (Sorted(ih,from, to)) return;

    SwapSyms(ih,pivot, (from + to) / 2); /* pivot on middle */
    
    while (pivot + 1 < ub) {
	if (ih->count[pivot + 1] < ih->count[pivot]) {
	    ub--;
	    SwapSyms(ih,pivot + 1, ub);
	} else {
	    SwapSyms(ih,pivot, pivot + 1);	
	    pivot++;
	}
    }
    Qs(ih,from, pivot);
    Qs(ih,pivot + 1, to);    
}

static void QSortSyms(IHISTOGRAM ih)
{
    Qs(ih,0, ih->nsyms);
}

void InitCounts1IHistogram(IHISTOGRAM ih, int *cnts)
{
  int i;
  
    ih->totalCount = 0;
    for (i = 0; i < ih->nsyms; i++) {
	ih->symToPos[i] = i;
	ih->posToSym[i] = i;
	ih->totalCount += cnts[i];
	ih->count[i] = cnts[i];
    }
    while (ih->totalCount >= ih->maxCt) { ScaleCountsIHistogram(ih); }
    QSortSyms(ih);
    BuildTreeCountIHistogram(ih);
}

void InitCounts2IHistogram(IHISTOGRAM ih, IHistBinFun *f, void *closure)
{
  int i;
  
    ih->totalCount = 0;
    f(closure, ih->count);
    for (i = 0; i < ih->nsyms; i++) {
	 ih->symToPos[i] = i;
	 ih->posToSym[i] = i;
	 ih->totalCount +=  ih->count[i];
    }
    while ( ih->totalCount >=  ih->maxCt) { ScaleCountsIHistogram(ih); }
    QSortSyms(ih);
    BuildTreeCountIHistogram(ih);
}

void IncCountIHistogram(IHISTOGRAM ih, int sym)
{
  int pos,parent;
  
    ReorganiseIHistogram(ih,sym);

    pos = ih->symToPos[sym];

    ih->count[pos]++;
    ih->totalCount++;

    while(pos > 0) {
	parent = Parent(pos);
	if (pos == Lchild(parent)) {
	    ih->treeCount[parent]++;
	}
	pos = parent;
    }

    if (ih->totalCount >= ih->maxCt) {
	ScaleCountsIHistogram(ih); 
	BuildTreeCountIHistogram(ih);
    }
}

int SymbolIHistogram(IHISTOGRAM ih, int val)
{
   int pos = 0;    
    int leftSum = 0;
    
    while(1) {

	if (leftSum + ih->treeCount[pos] > val) {
	    /* look at left subtree */
	    pos = Lchild(pos);
	} else {
	    if (val >= leftSum + ih->treeCount[pos] + ih->count[pos]) {
		/* look at right subtree */
		leftSum += ih->treeCount[pos] + ih->count[pos];
		pos = Rchild(pos);
	    } else {
		return ih->posToSym[pos];
	    }
	}
    }
}

float  EntropyIHistogram(IHISTOGRAM ih, int sym)
 { 
    return -log(CountIHistogram(ih,sym)/(float)ih->totalCount) * ih->onelog2; 
 }



