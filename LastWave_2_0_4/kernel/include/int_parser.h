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



extern char ParseNumLevel_(LEVEL level, char *arg, float *f, char flagTempAlloc, char flagSubst, char flagFast);

extern void ParseInt(char *arg, int *i);
extern char ParseInt_(char *arg, int defVal, int *i);
extern void ParseIntLevel(LEVEL level,char *arg, int *i);
extern char ParseIntLevel_(LEVEL level,char *arg, int defVal, int *i);

extern void ParseFloat(char *arg, float *i);
extern char ParseFloat_(char *arg, float defVal, float *i);
extern void ParseFloatLevel(LEVEL level,char *arg, float *i);
extern char ParseFloatLevel_(LEVEL level,char *arg, float defVal, float *i);

extern void ParseDouble(char *arg, double *i);
extern char ParseDouble_(char *arg, double defVal, double *i);
extern void ParseDoubleLevel(LEVEL level,char *arg, double *i);
extern char ParseDoubleLevel_(LEVEL level,char *arg, double defVal, double *i);

extern char *ParseFloatValLevel_(LEVEL level, char *arg, float *f, VALUE *val, unsigned char flagType, unsigned char listvElemType, char flagEmptySigIm);
extern char ParseValLevel__(LEVEL level, char *arg, VALUE defVal, VALUE *val,unsigned char flagType,unsigned char listvElemType, char flagEmptySI);
extern char ParseValLevel_(LEVEL level, char *arg, VALUE defVal, VALUE *val);
extern char ParseTypedValLevel_(LEVEL level, char *arg, VALUE defVal, VALUE *val, char *type);
extern void ParseValLevel(LEVEL level, char *arg, VALUE *val);
extern char ParseVal_(char *arg, VALUE defVal, VALUE *val);
extern void ParseVal(char *arg, VALUE *val);

extern char ParseValObjLevel_(LEVEL level, char *arg, VALUE defVal, VALUE *val);
extern void ParseValObjLevel(LEVEL level, char *arg, VALUE *val);
extern char ParseValObj_(char *arg, VALUE defVal, VALUE *val);
extern void ParseValObj(char *arg, VALUE *val);


extern void ParseChar(char *arg, char *c);
extern char ParseChar_(char *arg, char defVal, char *c);
extern void ParseCharLevel(LEVEL level, char *arg, char *c);
extern char ParseCharLevel_(LEVEL level, char *arg, char defVal, char *c);

extern void ParseWord(char *arg, char **str);
extern char ParseWord_(char *arg, char *defVal, char **str);

extern char ParseStrValueLevel_(LEVEL level, char *arg, STRVALUE defVal, STRVALUE *sc);
extern void ParseStrValueLevel(LEVEL level, char *arg, STRVALUE *sc);
extern void ParseStr(char *arg, char **str);
extern char ParseStr_(char *arg, char *defVal, char **str);
extern void ParseStrLevel(LEVEL level, char *arg, char **str);
extern char ParseStrLevel_(LEVEL level, char *arg, char *defVal, char **str);

extern char ParseFloatStrLevel_(LEVEL level,char *arg,float *f, char **str);

extern void ParseStream(char *arg, STREAM *stream);
extern char ParseStream_(char *arg, STREAM defVal, STREAM *stream);

extern void ParseLevel(char *arg, LEVEL *level);
extern char ParseLevel_(char *arg, LEVEL defVal, LEVEL *level);

/* A macro that defines characters that can be used for symbols */
#define IsValidSymbolChar1(c) (isalpha(c) || c == '_')  /* The first character */
#define IsValidSymbolChar(c) (isalnum(c) || c == '_')   /* The other ones */

extern char IsValidSymbol(char *name);
extern void ParseSymbol(char *arg, char **name);
extern char ParseSymbol_(char *arg, char *defVal, char **str);

extern void ParseColor(char *arg, unsigned long *color);
extern char ParseColor_(char *arg, unsigned long defVal, unsigned long *color);

extern void ParseColorMap(char *arg, unsigned long *color);
extern char ParseColorMap_(char *arg, unsigned long defVal, unsigned long *color);

extern void ParseWordList(char *theLine, char ***pList);
extern char ParseWordList_(char *theLine, char ** defList, char ***pList);
extern void ParseList(char *theLine, char ***pList);
extern char ParseList_(char *theLine, char ** defList, char ***pList);
extern void ParseListv(char *theLine, LISTV *pList);
extern char ParseListv_(char *theLine, LISTV defList, LISTV *pList);
extern void ParseListvLevel(LEVEL level,char *theLine, LISTV *pList);
extern char ParseListvLevel_(LEVEL level,char *theLine, LISTV defList, LISTV *pList);
extern char ParseListOrListv_(char *arg, char ***list, LISTV *lv);

extern char ParseRangeLevel_(LEVEL level, char *arg, RANGE defVal, RANGE *rg);
extern char ParseRange_(char *arg, RANGE defVal, RANGE *rg);
extern void ParseRangeLevel(LEVEL level, char *arg, RANGE *rg);
extern void ParseRange(char *arg, RANGE *rg);

extern char ParseStrScript_(char **theScript, SCRIPT defScript, SCRIPT *pScript);
extern void ParseStrScript(char **theScript, SCRIPT *pScript);
extern void ParseScript(char **theScript, SCRIPT *pScript);
extern char ParseScript_(char **theScript, SCRIPT defScript, SCRIPT *pScript);
extern void ParseScriptLevel(LEVEL level,char **theScript, SCRIPT *pScript);
extern char ParseScriptLevel_(LEVEL level,char **theScript, SCRIPT defScript, SCRIPT *pScript);
extern void ParseNoSubstScript(char **theScript, SCRIPT *pScript);
extern char ParseCompleteScript(char **theScript, SCRIPT *pScript,char flagBrace);

extern void ParseProc(char *arg, PROC *proc);
extern char ParseProc_(char *arg, PROC defProc, PROC *pProc);
extern void ParseProcLevel(LEVEL level, char *arg, PROC *proc);
extern char ParseProcLevel_(LEVEL level, char *arg, PROC defProc, PROC *pProc);


extern char ParseGObject_(char *arg, GOBJECT def, GOBJECT *obj);
extern void ParseGObject(char *arg, GOBJECT *obj);

extern char ParseGObjectList_(char *arg, GOBJECT *def, GOBJECT **obj);
extern void ParseGObjectList(char *arg, GOBJECT **objlist);

extern char ParseWindow_(char *arg, WINDOW def, WINDOW *w);
extern void ParseWindow(char *arg, WINDOW *w);

extern char ParseFont_(char *arg, FONT def, FONT *w);
extern void ParseFont(char *arg,  FONT *w);

extern char **ParseArgv(char **argv,...);
extern void NoMoreArgs(char **argv);
extern char ParseOption(char ***argv);
 
enum Types {  /* Optional types must be even numbers */

  MORE = -1,
  END = 0,
  
  tINT,tINT_,
  tFLOAT,tFLOAT_,
  tDOUBLE,tDOUBLE_,
  
  tVAL, tVAL_,
  tVALOBJ, tVALOBJ_,

  tCHAR,tCHAR_,    
  tSTR,tSTR_,
  tWORD,tWORD_,
  
  tSTREAM,tSTREAM_,

  tLEVEL,tLEVEL_,
  
  tSYMB,tSYMB_,
  tVNAME,tVNAME_,
  
  tCOLOR,tCOLOR_,
  tCOLORMAP,tCOLORMAP_,
      
  tLIST,tLIST_,
  tWORDLIST,tWORDLIST_,  
  tLISTV,tLISTV_,

  tRANGE,tRANGE_,
    
  tSCRIPT,tSCRIPT_,
  tPROC,tPROC_,
  
  tGOBJECT, tGOBJECT_,
  tGOBJECTLIST, tGOBJECTLIST_,

  tWINDOW, tWINDOW_,
  
  tFONT, tFONT_,
  
  tLAST

};


extern int AddParseTypeInt(char  (*function)(char *, int, int*));
extern int AddParseTypeFloat(char  (*function)(char *, float, float*));
extern int AddParseTypeChar( char  (*function)(char *, char, char*));
extern int AddParseTypePtr(char  (*function)(char *, void *, void **));
extern void EvalScriptLine(SCRIPTLINE sl, char flagHistory,char flagStoreResult);
extern char EvalScriptStringIfComplete(char *line,char flagHistory,char flagStoreResult);
extern void EvalScript(SCRIPT script,char flagStoreResult);
extern void EvalList(char **list, int nWords, char flagStoreResult);

