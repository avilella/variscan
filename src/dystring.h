/* dystring - dynamically resizing string. */


#ifndef DYSTRING_H	/* Wrapper to avoid including this twice. */
#define DYSTRING_H

struct dyString
/* Dynamically resizable string that you can do formatted
 * output to. */
    {
    struct dyString *next;	/* Next in list. */
    char *string;		/* Current buffer. */
    int bufSize;		/* Size of buffer. */
    int stringSize;		/* Size of string. */
    };

struct dyString *newDyString(int initialBufSize);
/* Allocate dynamic string with initial buffer size.  (Pass zero for default) */

#define dyStringNew newDyString

void freeDyString(struct dyString **pDs);
/* Free up dynamic string. */

#define dyStringFree(a) freeDyString(a);

void freeDyStringList(struct dyString **pDs);
/* Free up a list of dynamic strings */

#define dyStringFreeList(a) freeDyStringList(a);

void dyStringAppend(struct dyString *ds, char *string);
/* Append zero terminated string to end of dyString. */

void dyStringAppendN(struct dyString *ds, char *string, int stringSize);
/* Append string of given size to end of string. */

char dyStringAppendC(struct dyString *ds, char c);
/* Append char to end of string. */ 

void dyStringAppendMultiC(struct dyString *ds, char c, int n);
/* Append N copies of char to end of string. */ 

void dyStringVaPrintf(struct dyString *ds, char *format, va_list args);
/* VarArgs Printf to end of dyString. */

void dyStringPrintf(struct dyString *ds, char *format, ...);
/*  Printf to end of dyString.  Don't do more than 4000 characters this way... */

#define dyStringClear(ds) (ds->string[0] = ds->stringSize = 0)
/* Clear string. */

struct dyString * dyStringSub(char *orig, char *in, char *out);
/* Make up a duplicate of orig with all occurences of in substituted
 * with out. */

#endif /* DYSTRING_H */

