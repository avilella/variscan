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



/****************************************************************************/
/*                                                                          */
/*  history.h       This file contains the toplevel variables              */
/*                                                                          */
/****************************************************************************/




typedef struct struct_history {
  
  int size;
  int index;
  char **lines;
} *HISTORY;



extern void ReadHistory(HISTORY history,char *filename);
extern void WriteHistory(HISTORY history,char *filename);
extern void ClearHistory(HISTORY history);
extern void AllocHistory(HISTORY history,int size);
extern HISTORY NewHistory(void);
extern void DeleteHistory(HISTORY);
extern char *GetHistory(HISTORY history,int i);
extern void RecordHistory(char *line);

/* REMI : removed machine dependent definition of HISTORYFILE
 * and replaced it with the proper function
 */
extern char *GetHistoryFile(void);

