/*..........................................................................*/
/*                                                                          */
/*      L a s t W a v e   K e r n e l   2 . 0 . 1                           */
/*                                                                          */
/*      Copyright (C) 1998-2003 Emmanuel Bacry.                             */
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



#include "lastwave.h"




#define MaxNumOfPackages 60


static Package thePackages[MaxNumOfPackages];
static int nPackages = 0;


/*
 * Adding a package to lastwave
 */
void DeclarePackage(char *name,void (*loadFunction)(void),int year, char *version, char *authors, char *info)
{
  if (nPackages == MaxNumOfPackages-1)  Errorf("DeclarePackage() : Sorry, too many packages");
  thePackages[nPackages].load = loadFunction;
  thePackages[nPackages].flagLoaded = NO;
  thePackages[nPackages].authors = CopyStr(authors); 
  thePackages[nPackages].version = CopyStr(version); 
  thePackages[nPackages].year = year; 
  thePackages[nPackages].info = CopyStr(info); 
  thePackages[nPackages++].name = CopyStr(name); 
}


/*
 * Set the source directory where all the package directories are
 */
 
 static char *packageDir = NULL;
 
 void SetPackageDir(char *dir)
 {
   if (packageDir) Free(packageDir); 
   packageDir = CopyStr(dir);
 }  


/*
 * The Package command
 */
 
void C_Package(char **argv)
{
  char *actionName,*str,*authors,*info,*version;
  int i,year;
  char source[100],*list[2];
  LISTV lv,lv1;
  char *old;
  
  argv = ParseArgv(argv,tWORD,&actionName,-1);

  /* Case of the 'load' action */
  if (!strcmp(actionName,"load")) {
    argv = ParseArgv(argv,tSTR_,"*",&str,0);
    for (i=0;i<nPackages;i++) {
      if (MatchStr(thePackages[i].name,str)) {
        if (thePackages[i].flagLoaded == YES) {
           Printf("--- Package '%s' already loaded ...\n",thePackages[i].name);
        }
        else {
          old = toplevelCur->packageName;
          toplevelCur->packageName =  thePackages[i].name;
          Printf("--- Loading package '%s' %s (C) %d-2003 Copyright %s",thePackages[i].name,thePackages[i].version,thePackages[i].year,thePackages[i].authors);
          if (thePackages[i].load != NULL) (*(thePackages[i].load))();
          Printf("\n");
          toplevelCur->packageName = old;
          thePackages[i].flagLoaded = YES;
          sprintf(source,"%s/%s",packageDir,thePackages[i].name);
          AddSourceDir(source);
        }
        sprintf(source,"%s.pkg",thePackages[i].name);
        list[0] = source;
        list[1] = NULL;
        Source(list);
      }
    }
    return;
  }
  
  /* Case of 'name' action */
  if (!strcmp(actionName,"name")) {
    argv = ParseArgv(argv,tSTR_,"*",&str,0);
    SetResultStr("");
    for (i=0;i<nPackages;i++) {
      if (MatchStr(thePackages[i].name,str)) 
        AppendListResultStr(thePackages[i].name);
    }
    return;
  }

  /* Case of 'new' action */
  if (!strcmp(actionName,"new")) {
    argv = ParseArgv(argv,tSTR,&str,tINT,&year,tSTR,&version,tSTR,&authors,tSTR,&info,0);
    SetResultStr("");
    for (i=0;i<nPackages;i++) {
      if (!strcmp(thePackages[i].name,str)) Errorf("Sorry this package name is already used"); 
    }
    DeclarePackage(str,NULL,year,version,authors,info);
    return;
  }
 
  /* Case of 'dir' action */
  if (!strcmp(actionName,"dir")) {
    argv = ParseArgv(argv,tSTR_,NULL,&str,0);
    SetResultStr("");
    if (str) SetPackageDir(str);
    else SetResultStr(packageDir);
    return;
  }
    
  /* Case of 'list' action */
  if (!strcmp(actionName,"list")) {
    argv = ParseArgv(argv,tSTR_,"*",&str,0);
    lv = TNewListv();
    SetResultStr("");
    for (i=0;i<nPackages;i++) {
      if (MatchStr(thePackages[i].name,str)) {
        lv1 = TNewListv();
        AppendValue2Listv(lv,(VALUE) lv1);
        AppendStr2Listv(lv1,thePackages[i].name);
        AppendInt2Listv(lv1,thePackages[i].flagLoaded);
        AppendInt2Listv(lv1,thePackages[i].year);
        AppendStr2Listv(lv1,thePackages[i].version);
        AppendStr2Listv(lv1,thePackages[i].authors);
        AppendStr2Listv(lv1,thePackages[i].info);
      }
    }
    SetResultValue(lv);
    return;
  }

  SetErrorf("Bad action '%s'\n",actionName); 
  ErrorUsage1();
}
