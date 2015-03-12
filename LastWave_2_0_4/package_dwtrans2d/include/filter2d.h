/*..........................................................................*/
/*                                                                          */
/*      L a s t W a v e    P a c k a g e 'dwtrans2d' 2.0                    */
/*                                                                          */
/*      Copyright (C) 1998-2002  E.Bacry, J.Fraleu, J.Kalifa, E. Le Pennec, */
/*                         W.L. Hwang , S.Mallat, S.Zhong                   */
/*      emails : lastwave@cmap.polytechnique.fr                             */
/*               fraleu@cmap.polytechnique.fr                               */
/*               kalifa@cmap.polytechnique.fr                               */
/*               lepennec@cmap.polytechnique.fr                             */
/*               mallat@cmap.polytechnique.fr                               */
/*               whwang@iis.sinica.edu.tw                                   */
/*               szhong@chelsea.princeton.edu                               */
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


#ifndef FILTER2D_H

#define FILTER2D_H

/********************************************************************/
/***************************** FILTERS ******************************/
/********************************************************************/


#define W2_FILT_SIZE 128 /* max size of a filter */

typedef struct filter2 {    
  int size;
  int shift;		/* shift half grid point to left: 1; to right: -1. */
  float symmetry;	/* symmetry: 1.0; antisymmetry: -1.0. */
  float values[W2_FILT_SIZE];
  char * name;
  float factors[W2_NFACT];
} Filter2, *FILTER2;


/***********************************************************/
/*******************    filter       ***********************/
/***********************************************************/

/*filter_alloc2d.c */
extern FILTER2 NewFilter2(void);
extern void DeleteFilter2(FILTER2 filter);
extern void ClearFilter2(FILTER2 filter);
extern void SetDefaultFilter2Name(char *str);  
extern char  *GetDefaultFilter2Name(void);  
extern void InitDFilter2(void);

#endif
