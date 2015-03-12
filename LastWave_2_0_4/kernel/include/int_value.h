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
/*  int_value.h       This file contains the VALUE definition   */
/*                                                                          */
/****************************************************************************/


#ifndef VALUE_H

#define VALUE_H 


/* 
 * Abstract structure for the content of a variable
 *
 *   Any structure you want to be a variable should start with the
 *   following fields.
 *  
 * WARNING : If you change it, you should change the definition in int_hash.h
 */

#define ValueFields \
  struct typeStruct *ts; \
  void * (*sendMessage) (void *content,int message,void **arg); \
  short nRef 

typedef struct value {

  ValueFields;
  
} *VALUE;


/*
 * The method list structure to access fields
 */

typedef struct field {
  
  char *name;

  /* 
   * The Get function : void *Get(struct value * vc, void **arg);
   *
   * arg[0] = fieldName (NULL if extraction on the object itself)
   * arg[1] = The eventual FSIList for extraction
   * arg[2] = a pointer to the result if float
   * arg[3] = a pointer to the result if string 
   * arg[4] = a pointer to the result if not a float
   *
   * If an VALUE is returned (in arg[4]), you should manage the total nRef.
   * Thus if it is a VALUE that you created just to answer the message
   * you should just make it temporary. If you had the VALUE before, just 
   * add 1 reference and make it temporary
   *
   * Same thing for strings : if it is a string that was already allocated, you
   * don't need to do anything otherwise you have to make it temporary
   *
   * Returns NULL if field does not exist (use SetErrorf to set the error).
   * Otherwise, returns the type of the result.
   */  
  void *Get;

  /* 
   * The Set function : void *Set(struct value * vc, void **arg); 
   *
   * arg[0] = fieldName to be set
   * arg[1] = The eventual FSIList associated to this field
   * arg[2] = a pointer to the value if float
   * arg[3] = a pointer to the value if string
   * arg[4] = a pointer to the value if not a string nor a float
   * arg[5] = a pointer to a char which indicates whether := was used or not
   * Returns NULL if field does not exist (use SetErrorf to set the error)
   */
  void *Set;
  
  /* 
   * The ExtractOptions function : void *ExtractOptions(struct value * vc, void **arg); 
   *
   * arg[0] = fieldName (NULL if extraction on the object itself)
   * Returns the options (NULL terminated array of strings, e.g. *nolimit,...)  associated to 
   * the specified field. 
   * returns NULL if no extraction is available
   */ 
  void *ExtractOptions;

  /* 
   * The ExtractInfo function : void *ExtractInfo(struct value * vc, void **arg); 
   *
   * arg[0] = fieldName (NULL if extraction on the object itself)
   * arg[1] = &options (a pointer to the options mask --> type is unsigend long *)
   * Returns the info structure (ExtractInfo *) associated to the field and the
   * options (fieldName can be NULL if extraction of the object itself is performed)
   * Returns NULL if no extraction is available
   */  
  void *ExtractInfo;  

} Field;

/*
 * The type structure of a VALUE
 */

typedef struct typeStruct {
  
  /* The documentation on this type */
  char *doc;

  /* The basic (unique) type name */
  char **type;
  
  /* The GetType function */                       
  char * (*GetType)(struct value *vc); 

  /* The Delete function : void Delete(VALUE val) */
  void *Delete;   

  /* The New function : VALUE New(void) */
  void *New;   

  /* The Copy function : VALUE Copy (VALUE in, VALUE out) */
  void *Copy;   

  /* The Clear function void Clear(VALUE in) */
  void *Clear;   
  
  /* Convert the object into a string "short" or "normal" : char *ToStr(VALUE val, char flagShort) */
  void *ToStr;   

  /* The LongPrint function : print the object when 'print' is called : void Print(VALUE val) */
  void *Print;

  /* The PrintInfo function : called by 'info' : void PrintInfo(VALUE val) */
  void *PrintInfo;

  /* 
   * The NumExtract function : used to deal with syntax like 10a : void NumExtract(VALUE val, void **arg)
   *
   * arg[0] = an integer
   * arg[1] = a flag that indicates whether a '.' was find 
   * arg[2] = a pointer to the result if float
   * arg[3] = a pointer to the result if string (should be allocated)
   * arg[4] = a pointer to the result if not a string or a float
   * Returns NULL if no extraction is available
   */ 
  void *NumExtract;

  /* The fields */
  Field *fields;

} TypeStruct, *TYPESTRUCT;


/* Types which are used only for procedure argument types */
extern char *wordType;
extern char *wordlistType;
extern char *valType;
extern char *valobjType;




/* A list just for compatibility with LastWave 1.xxx.... will be deleted soon */
enum {
  DeleteMsge = 1,
  GetTypeMsge,
  GetNameMsge,
  PrintMsge,
  LongPrintMsge,
  ShortPrintMsge,
  PrintInfoMsge,
  NumExtractMsge,
  GetExtractOptionsMsge,
  GetExtractInfoMsge,
  GetFieldMsge,
  SetFieldMsge,
  ExtractMsge,
  DisplayMsge,
  ToStrMsge,    
  SendNameMsge,
  GetImageMsge,
  GetSignalMsge,
  GetValueMsge
};


extern Field *FindField(Field *fields, char *name);
extern void AddRefValue_(VALUE value);
extern void RemoveRefValue_(VALUE value);
extern void TempValue_(VALUE value);
extern char DoesTypeOverwrite(char *type, char *typeOverwrite);
extern char *GetBTypeContent(VALUE vc);

#define TempValue(value) TempValue_((VALUE) value)
#define AddRefValue(value) AddRefValue_((VALUE) value)
#define RemoveRefValue(value) RemoveRefValue_((VALUE) value)

extern void *GetValue_(VALUE val, void **arg);
#define GetValueField(val,arg) GetValue_((VALUE) val,arg)

extern char *varType;

#define InitValue(vc,ts1) vc->ts = ts1; vc->sendMessage = NULL; vc->nRef = 1

#define IsVariable(vc) ((vc)->ts != NULL && *((vc)->ts->type) == varType)

#define ValueOf(value) (!(IsVariable(value)) ? (value) : *GetVariablePContent((VARIABLE) (value),NO))

#define GetTypeValue(vc) \
  (((vc)->ts == NULL ? ((*((vc)->sendMessage))(vc,GetTypeMsge,NULL)) : \
   ((vc)->ts->GetType != NULL ? (*((vc)->ts->GetType))((VALUE) vc) : *((vc)->ts->type))))

#define GetTrueTypeValue(vc) \
  (((vc)->ts == NULL ? GetTypeValue(vc) : *((vc)->ts->type)))

#define DeleteValue(vc) \
  if ((vc)->ts == NULL) (*((vc)->sendMessage))(vc,DeleteMsge,NULL); else  ((void * (*)(VALUE)) ((vc)->ts->Delete)) ((VALUE) vc)

#define NewValue(ts) ((VALUE (*)(void)) (ts->New))()

#define ToStrValue(vc,flagShort) \
  ((vc)->ts == NULL ?  "" : \
    ((char * (*)(struct value *, char)) ((vc)->ts->ToStr)) ((VALUE) vc,flagShort))

#define PrintValue(vc) \
  if ((vc)->ts == NULL) (*((vc)->sendMessage))(vc,LongPrintMsge,NULL); \
  else if ((vc)->ts->Print == NULL) Printf("%s\n",ToStrValue(vc,NO)); \
  else (*((void (*)(struct value *)) ((vc)->ts->Print))) ((VALUE) vc)
  
#define PrintInfoValue(vc) \
  if ((vc)->ts == NULL) (*((vc)->sendMessage))(vc,PrintInfoMsge,NULL); \
  else (*((void (*)(struct value *)) ((vc)->ts->PrintInfo))) ((VALUE) vc)

#define NumExtractValue(vc,arg) \
  (((vc)->ts == NULL ? ((*((vc)->sendMessage))(vc,NumExtractMsge,arg)) : \
   ((vc)->ts->NumExtract == NULL ? NULL : \
  (*((void * (*)(struct value *,void **)) ((vc)->ts->NumExtract))) ((VALUE) (vc),(void **) (arg)))))

#define NumExtractTS(ts,arg) \
  (*((void * (*)(VALUE,void **)) (ts)->NumExtract))(NULL,arg)

#define GetExtractOptionsValue(vc,arg) \
  (((vc)->ts == NULL ? ((*((vc)->sendMessage))(vc,GetExtractOptionsMsge,arg)) : \
   (FindField((vc)->ts->fields,arg[0]) == NULL ? NULL : \
    (FindField((vc)->ts->fields,arg[0])->ExtractOptions == NULL ? NULL : \
     ((void * (*)(struct value *, void **)) (FindField((vc)->ts->fields,arg[0]))->ExtractOptions)((VALUE) vc,arg)))))

#define GetExtractInfoValue(vc,arg) \
  (((vc)->ts == NULL ? ((*((vc)->sendMessage))(vc,GetExtractInfoMsge,arg)) : \
   (FindField((vc)->ts->fields,arg[0]) == NULL ? NULL : \
   ((void * (*)(struct value *, void **)) (FindField((vc)->ts->fields,arg[0]))->ExtractInfo)((VALUE) vc,arg))))


#define GetFieldValue(field,val,arg) (*((void * (*)(VALUE,void **)) (field).Get))(val,arg)
#define SetFieldValue(field,val,arg) (*((void * (*)(VALUE,void **)) (field).Set))(val,arg)
#define ExtractOptionFieldValue(field,val,arg) (*((void * (*)(VALUE,void **)) (field).ExtractOptions))(val,arg)
#define ExtractInfoFieldValue(field,val,arg) (*((void * (*)(VALUE,void **)) (field).ExtractInfo))(val,arg)

#ifdef NUMDOUBLE
#define Flt2Str(flt,str) sprintf(str,"%.16g",flt)
#else
#define Flt2Str(flt,str) sprintf(str,"%.8g",flt)
#endif

#define CastValue(value,type) ((type) (ValueOf(value)))

#define ARG_EO_GetField(arg) ((char *) (arg)[0])

#define ARG_EI_GetField(arg) ((char *) (arg)[0])
#define ARG_EI_GetPOptions(arg) ((unsigned long *) (arg)[1])

#define ARG_S_GetField(arg) ((char *) (arg)[0])
#define ARG_S_SetField(arg,field) ((char *) (arg)[0])=(field)
#define ARG_S_GetFsiList(arg) ((FSIList *) (arg)[1])
#define ARG_S_SetFsiList(arg,fsi) ((FSIList *) (arg)[1])=(fsi)
#define ARG_S_GetRightType(arg) ((char *) (arg)[2])
#define ARG_S_GetRightFloat(arg) (*((float *) (arg)[3]))
#define ARG_S_GetRightValue(arg) (*((VALUE *) (arg)[4]))
#define ARG_S_GetEqual(arg) ((char *) (arg)[5])
#define ARG_S_GetResPValue(arg) ((VALUE *) (arg)[6])
#define ARG_S_SetResValue(arg,val) (*((VALUE *) (arg)[6]) = (val))
#define ARG_S_GetResPFloat(arg) ((float *) (arg)[7])
#define ARG_S_GetResPStr(arg) ((char **) (arg)[8])

#define ARG_G_GetField(arg) ((char *) (arg)[0])
#define ARG_G_SetField(arg,field) ((char *) (arg)[0])=(field)
#define ARG_G_GetFsiList(arg) ((FSIList *) (arg)[1])
#define ARG_G_GetResPFloat(arg) ((arg)[2])
#define ARG_G_SetResFloat(arg,flt) (*((float *) (arg)[2]) = (flt))
#define ARG_G_GetResPStr(arg) ((arg)[3])
#define ARG_G_SetResStr(arg,str) (*((char **) (arg)[3]) = (str))
#define ARG_G_GetResPValue(arg) ((arg)[4])
#define ARG_G_SetResValue(arg,val) (*((VALUE *) (arg)[4]) = (VALUE) (val))

#define ARG_NE_GetResPValue(arg) ((arg)[4])
#define ARG_NE_GetN(arg) ((int) *((float *) (arg)[0]))
#define ARG_NE_GetFlagDot(arg) (*((char *) (arg)[1]))
#define ARG_NE_SetResValue(arg,val) (*((VALUE *) (arg)[4]) = (VALUE) (val))

/************************************************************
 *
 *
 *  basic VALUE types
 *
 *
 ************************************************************/


/***********************
 *
 * The 'null' value 
 *
 ***********************/
extern VALUE NewNullValue(void);
extern VALUE nullValue;
extern char *nullType;


/***********************
 *
 * The STRVALUE 
 *
 ***********************/
 
typedef struct strValue {

  /* The fields of VALUE */ 
  ValueFields;

  char *str; /* the string */
  char **list;  /* the eventual associated list */

} StrValue, *STRVALUE;
 
extern char * strType;
extern char * listType;
extern TypeStruct tsStr;

/* The minimum size of a string (should be greater than what is needed to represent a number) */
#define MinStringSize 30 

/* (des)allocation */
extern void DeleteStrValue(STRVALUE sc);
extern void InitNullStrValue(STRVALUE sc);
extern void InitStrValue(STRVALUE sc);
extern char * StrValueStrAlloc(int n);
extern STRVALUE NewNullStrValue(void);
extern STRVALUE NewStrValue(void);
extern STRVALUE TNewStrValue(void);

/* Routins to Get/Set string fields */
extern void *SetStrField(char **pstr, void **arg);
extern void *GetStrField(char *str, void **arg);

/* Simple operations */
extern void CopyStrValue(STRVALUE in, STRVALUE out);
extern void ConcatStrValue(STRVALUE sc1,STRVALUE sc2,STRVALUE sc3);
extern void MultStrValue(STRVALUE sc1, int n, STRVALUE sc3);

extern char *GetStrFromStrValue(STRVALUE sc);
extern char **GetListFromStrValue(STRVALUE sc);
extern void SetStrValue(STRVALUE sc, char *str);

/* Managing Variables */
struct level;
extern char * GetStrVariableLevel(struct level *level,char *name);
extern char * GetStrVariable(char *name);
extern void SetStrVariableLevel(struct level * level,char *name,char *value);
extern void SetStrVariable(char *name,char *value);
extern void SetStrVariablef(char *name,char *format, ...);
extern void SetStrVariableListLevel(struct level *level,char *name,char **list, char flagBracket);
extern void SetStrVariableList(char *name,char **list, char flagBracket);


/* Managing regexp */
extern char MatchStr(char *str,char *expr);
extern struct listvValue  *MatchStrN(char *str,char *expr, int nOcc);

/* Basic routines on strings */
extern void DeleteStr(char *str);
extern char *CopyStr(char *str);
extern char *TCopyStr(char *str);
extern void TempStr(char *str);
extern void PrintStrColumn(char *str,int size);

/* Basic routines on lists */
extern void DeleteList(char **list);
extern char **CopyList(char **list);
extern char **TCopyList(char **list);
extern char **Array2List(char **array, int nElem);
extern char **TArray2List(char **array, int nElem);
extern char *List2Str(char **list,char *separator);
extern char *TList2Str(char **list,char *separator);
extern char **Str2List(char *str,...);
extern char **TStr2List(char *str,...);
extern char **BegEndStr2List(char *begStr[],char *endStr[]);
extern char **TBegEndStr2List(char *begStr[],char *endStr[]);
extern char IsList(char *str);
extern int GetListSize(char **list);


/***********************
 *
 * The NUMVALUE 
 *
 ***********************/

/* 
 * The structure of a num variable content
 */
typedef struct numValue {

  /* The fields of VALUE */ 
  ValueFields;

  float f; /* the float */

} NumValue, *NUMVALUE;
 
extern char * numType;
extern char * intType;
extern char * floatType;
extern TypeStruct tsNum;


extern void DeleteNumValue(NUMVALUE nc);
extern void InitNumValue(NUMVALUE nc);
extern NUMVALUE NewNumValue(void);
extern NUMVALUE TNewNumValue(void);
extern void CopyNumValue(NUMVALUE in, NUMVALUE out);

extern void SetNumValue(NUMVALUE nc, float value);

extern int GetIntVariableLevel(struct level * level,char *name);
extern int GetIntVariable(char *name);
extern float GetFloatVariableLevel(struct level * level,char *name);
extern float GetFloatVariable(char *name);
extern void SetNumVariableLevel(struct level * level,char *name,float value);
extern void SetNumVariable(char *name,float value);


/* Functions to Get/Set number fields of objects */
#define FieldPositive 1
#define FieldSPositive 2
#define FieldNegative 3
#define FieldSNegative 4
extern void *SetFloatField(float *pflt, void **arg, char flag);
extern void *SetIntField(int *pint, void **arg,char flag);
extern void *GetFloatField(float f, void **arg);
extern void *GetIntField(int i, void **arg);


/***********************
 *
 * The LISTV 
 *
 ***********************/


typedef struct listvValue {

  /* The fields of VALUE */ 
  ValueFields;

  int nAlloc;             /* The allocated number of elements */
  int length;             /* The effective number of elements */
  float *f;               /* float elements */
  VALUE *values;              /* the value elements (when an element is NULL then it means that we should look into f[] */

} * LISTV;

/* The corresponding type name */
extern char *listvType;
extern TypeStruct tsListv;

extern LISTV NewListv(void);
extern LISTV TNewListv(void);
extern void DeleteListv(LISTV lv);
extern void SetLengthListv(LISTV lv, int length);
extern void ClearListv(LISTV lv);

extern char *GetListvNth(LISTV lv,int i,VALUE *vc,float *f);
extern char *GetListvNthStr(LISTV lv,int i);
extern float GetListvNthFloat(LISTV lv,int n);

extern void SetListvNthFloat(LISTV lv,int n,float f);
extern void SetListvNthValue(LISTV lv,int n,VALUE vc);

extern void *SetListvField(LISTV *plv,void **arg);

extern int AreEqualListv(LISTV lv1,LISTV lv2);

extern LISTV CopyListv(LISTV in,LISTV out);
extern void ConcatListv(LISTV lv1,LISTV lv2,LISTV lv3);
extern void MultListv(LISTV lv1, int n, LISTV l3);

extern void AppendValue2Listv (LISTV lv, VALUE vc);
extern void AppendFloat2Listv (LISTV lv, float f);
extern void AppendFloatArray2Listv (LISTV lv, float *f, int n);
extern void AppendInt2Listv (LISTV lv, int i);
extern void AppendIntArray2Listv (LISTV lv, int *i, int n);
extern void AppendStr2Listv (LISTV lv, char *str);
extern void AppendStr2Listvf(LISTV lv, char *format,...);
extern void AppendListv2Listv (LISTV lv, LISTV lv1);




/* 
 * The structure of a range variable content
 *   It should start with the fields of VALUE
 */

typedef struct rangeValue {

  /* The fields of VALUE */ 
  ValueFields;

  float first,step;
  int size;

} * RANGE;

/* The corresponding type name */
extern char *rangeType;
extern TypeStruct tsRange;

extern RANGE NewRange(void);
extern RANGE TNewRange(void);
extern void DeleteRange(RANGE r);

#define RangeVal(r,i) ((r)->step*(i)+(r)->first)
#define RangeLast(r) (RangeVal((r),(r)->size-1))
#define RangeFirst(r) ((r)->first)
#define RangeMax(r) ((r)->step>0 ? RangeLast(r) : RangeFirst(r))
#define RangeMin(r) ((r)->step>0 ? RangeFirst(r) : RangeLast(r))



#endif
