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
/*  int_history.c    Functions to deal with the interpreter history         */
/*                                                                          */
/****************************************************************************/



#include "lastwave.h"

/* ANSI changed */
#include "xx_system.h"


#define HistoryMinLineLength 30


/***********************************************************
 *
 * (Des)Allocation of History structures
 *
 ***********************************************************/

 
/*
 * Clear the content of an history
 */

void ClearHistory(HISTORY history)
{
  int i;

  if (history->size == 0) return;
  
  if (history->lines != NULL) {
    for (i=0;i<history->size;i++) if (history->lines[i] != NULL) Free(history->lines[i]);
    Free(history->lines);
  }

  history->size = 0;
}

/*
 * Delete an history
 */

void DeleteHistory(HISTORY history)
{
  ClearHistory(history);

  Free(history);
}

/*
 * Allocation of an history
 */

HISTORY NewHistory(void)
{
  HISTORY h;
  
  h =  (HISTORY) Malloc(sizeof(struct struct_history));
  h->size = 0;
  h->lines = NULL;
 
  return(h);
}


/*
 * (Re)Allocation of the content of an history
 */

void AllocHistory(HISTORY history,int size)
{
  int i;

  ClearHistory(history);

  history->lines =  (char * *) Malloc(sizeof(char *)*size);
  for (i=0;i<size;i++) {
    history->lines[i] = (char *) Malloc(sizeof(char)*(HistoryMinLineLength+1));
    history->lines[i][0] = '\0';
  }
  history->size = size;
  history->index = 0;
}

/*
 * Copy a line in the history at index k
 */

static void SetHistoryLine(HISTORY h, char *str, int k)
{
  int l;
  
  if (str == NULL) Errorf("SetHistoryLine() : Weird error");
  if (k < 0 || k >= h->size) 
      Errorf("SetHistoryLine() : Weird error");
  while (*str == ' ') str++;
  
  l = strlen(str);
  if (l < HistoryMinLineLength || l <= strlen(h->lines[k])) strcpy(h->lines[k],str);
  else {
    Free(h->lines[k]);
    h->lines[k] = CopyStr(str);
  }
}

   
/*
 * Change the size of an history (without loosing its content)
 */

void ChangeHistorySize(HISTORY history,int newSize)
{
  int i,j,k;
  struct struct_history oldHist;

  if (history->size == newSize) return;

  oldHist.size = history->size;
  oldHist.lines = history->lines;
  oldHist.index = history->index;

  history->size = 0;
  history->lines = NULL;
  
  AllocHistory(history,newSize);

  for (i = newSize-2;i>=0;i--) {
    j = (oldHist.index +1 + i - newSize+100*oldHist.size)%oldHist.size;
    k = (i-newSize+1+oldHist.index+10*newSize)%newSize;
    if (j == oldHist.index%oldHist.size) break;
    SetHistoryLine(history,oldHist.lines[j],k);
  }

  history->index = oldHist.index;
  ClearHistory(&oldHist);
}


/***********************************************************
 * 
 * History file management
 *
 ***********************************************************/

/*
 * Read an history file
 */

void ReadHistory(HISTORY history,char *filename)
{
  FILE *fp;
  int i,size;
  char maxLine[2000];
  

  if (filename != NULL && ((fp = FOpen (filename, "r")) != NULL)) {

    if (fscanf(fp,"%d\n",&size) != 1) {
      FClose(fp);
      Warningf("Unable to read history file '%s'\n",filename);
      AllocHistory(history,10);
      return;
    }
    
    if (size == 0) {
      AllocHistory(history,10);
      return;
    }
    
    AllocHistory(history,size);
    
    for(i = 0; i < size; ++i) {
       fgets(maxLine, 2000, fp);
       if (maxLine[strlen(maxLine)-1] == '\n' || maxLine[strlen(maxLine)-1] == '\r') maxLine[strlen(maxLine)-1] = '\0';
       SetHistoryLine(history,maxLine,i);
       if (feof(fp)) break;
    }

    FClose(fp);

    history->index = MIN(i+1,size);
  }
  else {
    AllocHistory(history,10);
  }
}


/*
 * Write the history in a file
 */

void WriteHistory(HISTORY history,char *filename)
{
  FILE *fp;
  int i;

  if (filename != NULL && ((fp = FOpen (filename, "w")) != NULL)) {

    fprintf(fp,"%d\n",history->size);

    for(i = MAX(history->index-history->size,0); i <= history->index-1; ++i) 
      fprintf(fp,"%s\n",history->lines[i % history->size]);

    FClose(fp);
  }
}



/***********************************************************
 * 
 * Main functions for managing histories
 *
 ***********************************************************/

/*
 * Get an history line (assocaieted to number 'i')
 */
char *GetHistory(HISTORY history,int i)
{
  if (i > 0 && INRANGE(history->index - history->size+1, i, history->index)) 
    return(history->lines[(i-1) % history->size]);
  else Errorf("This command number doesn't exist!");
}

/* ANSI changed Get history file */
char *GetHistoryFile()
{
  return(XXGetHistoryFile());
}

/*
 * This function is called after a  command-line has been typed.  
 * It pushes the command-line 'line' in the 'history' stack     
 */

void RecordHistory(char *line)
{
  HISTORY history = toplevelCur->history;

  SetHistoryLine(history,line,history->index%history->size);
  history->index++;  
  toplevelCur->levels[0].cmdNum++;
}
 
 
/***********************************************************
 * 
 * Main command for managing histories
 *
 ***********************************************************/
    
void C_History(char **argv)
{
  int i;
  HISTORY h;
  char *command,*regexp;
  LISTV lv;
  
  h = toplevelCur->history;
  
  /* Case we just want to display the history */
  if (*argv == NULL) {
    for(i = MAX(h->index-h->size,0); i <= h->index-1; ++i) 
      Printf("%3d  %s\n", i+1, h->lines[i % h->size]);
    return;
  }
  
  argv = ParseArgv(argv,tWORD,&command,-1);
  
  /* Case we want to get the line number associated to a given number */
  if (!strcmp(command,"index")) {
    if (*argv == NULL) SetResultInt(levelFirst->cmdNum);
    else {
      argv = ParseArgv(argv,tINT,&i,0);
      SetResultStr(GetHistory(h,i));
    }
  }
  
  /* Case we want to perform a completion */
  else if (!strcmp(command,"match")) {
    argv = ParseArgv(argv,tSTR_,"*",&regexp,0);
    h = toplevelCur->history;
    lv = TNewListv();
    for (i= h->index-1;i>=h->index-h->size+1;i--) {
      if (MatchStr(h->lines[(i+h->size)%h->size],regexp)) 
        AppendStr2Listv(lv,h->lines[(i+h->size)%h->size]);
    }      
    SetResultValue(lv);
    return;
  }
  
  /* Case we want to change the size of the history */
  else if (!strcmp(command,"size")) {
    if (*argv == NULL) SetResultInt(h->size);
    else {
      argv = ParseArgv(argv,tINT,&i,0);
      ChangeHistorySize(toplevelCur->history,i);
    }
  }
  
  else Errorf("Unknown command '%s'",command);
}
