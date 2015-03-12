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



extern char *_theDoc;
#define DOC(str) {_theDoc = str; return;}


/**********************************************************/
/* extern function  defined in the file int_main.c        */
/**********************************************************/

extern int GetPrompt(char **pPrompt);

/**********************************************************/
/* extern function  defined in the file int_alloc.c      */
/**********************************************************/

extern void *Malloc(size_t size);
extern void * TMalloc(size_t size);
extern void *Realloc(void *ptr, size_t size);
extern void *Calloc(int n, size_t size);
extern char *CharAlloc(int size);
extern float *FloatAlloc(int size);
extern double *DoubleAlloc(int size);
extern int *IntAlloc(int size);
extern void Free(void * ptr);

extern void TempPtr(void *ptr);
extern void TempList(char **list);
extern void SetTempAlloc(void);
extern void ClearTempAlloc(void);
extern void ClearAllTempAlloc(void);
											

/**********************************************************/
/* extern function  defined in the file int_errors.c      */
/**********************************************************/

extern void InitError();
extern void Warningf(char *format,...);
extern void ErrorOption(char c);
extern void ErrorUsage(void);
extern void ErrorUsage1(void);
extern void Errorf(char *format,...);
extern void Errorf1(char *format,...);
extern void SyntaxError(char msge[],char *line, char *ptr);
extern void SetErrorf(char *format,...);
extern void AppendErrorf(char *format,...);


/*********************************************************
 *
 * extern function  to manage results
 * defined in int_error_result.c   
 *
 **********************************************************/

extern void InitResult();
extern void SetResultf(char *format,...);
extern void SetResultStr(char *str);
extern void SetResultList(char **list);
extern void AppendResultStr(char *str);
extern void AppendResultf(char *format,...);
extern void AppendListResultStr(char *str);
extern void AppendListResultf(char *format,...);
extern void SetResultFloat(float f);
extern void SetResultContent_(struct value *val);
#define SetResultValue(val) SetResultContent_((VALUE) (val))
extern void SetResultInt(int f);

extern char *GetResultType(void);
extern struct value *GetResultValue(void);
extern char * GetResultStr(void);
extern int GetResultInt(void);
extern float GetResultFloat(void);

extern void PrintResult(void);


/******** */
extern FILE * FOpen(char *file, char * mode);
extern int FClose(FILE *s);


/* In int_expr.c */
extern char *TTEvalExpressionLevel_(LEVEL level,char* arg, float *resFloat, struct value **resVC,  unsigned char flagType, char flagSubst, char flagFast,unsigned char listvElemType, char flagEmptySI);



/* In commands_user.c */

extern void UserInit(void);


/* In int_package.c */
extern void DeclarePackage(char *name,void (*load)(void),int year,char *version, char * authors,char *info);
extern void SetPackageDir(char *dir);




