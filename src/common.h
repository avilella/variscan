/* Common.h - functions that are commonly used.  Includes 
 * routines for managing singly linked lists, some basic
 * string manipulation stuff, and other stuff of the
 * short but useful nature. 
 *
 * This file is copyright 2002 Jim Kent, but license is hereby
 * granted for all use - public, private or commercial. */

#ifndef COMMON_H	/* Wrapper to avoid including this twice. */
#define COMMON_H

/* Some stuff to support large files in Linux. */
#ifndef _LARGEFILE_SOURCE
#define _LARGEFILE_SOURCE 1
#endif

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#ifndef _FILE_OFFSET_BITS
#define _FILE_OFFSET_BITS 64
#endif

/* Some stuff for safer pthreads. */
#ifndef _REENTRANT
#define _REENTRANT
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <fcntl.h>
#include <assert.h>
#include <setjmp.h>
#include <time.h>
#include <math.h>
#include <errno.h>
#include <unistd.h>

/* Let's pretend C has a boolean type. */
#define TRUE 1
#define FALSE 0
#define boolean int
#define bool char

/* Some other type synonyms */
#define UBYTE unsigned char   /* Wants to be unsigned 8 bits. */
#define BYTE signed char      /* Wants to be signed 8 bits. */
#define UWORD unsigned short  /* Wants to be unsigned 16 bits. */
#define WORD short	      /* Wants to be signed 16 bits. */
#define bits32 unsigned       /* Wants to be unsigned 32 bits. */
#define bits16 unsigned short /* Wants to be unsigned 16 bits. */
#define bits8 unsigned char   /* Wants to be unsigned 8 bits. */
#define signed32 int	      /* Wants to be signed 32 bits. */
#define bits8 unsigned char   /* Wants to be unsigned 8 bits. */

#define BIGNUM 0x3fffffff	/* A really big number */

/* Default size of directory path string buffers */
#define PATH_LEN 512

/* How big is this array? */
#define ArraySize(a) (sizeof(a)/sizeof((a)[0]))

#define uglyf printf  /* debugging printf */
#define uglyAbort errAbort /* debugging error abort. */
#define uglyOut stdout /* debugging fprintf target. */

void *needMem(size_t size);
/* Need mem calls abort if the memory allocation fails. The memory
 * is initialized to zero. */

void *needLargeMem(size_t size);
/* This calls abort if the memory allocation fails. The memory is
 * not initialized to zero. */

void *needLargeZeroedMem(long size);
/* Request a large block of memory and zero it. */

void *needHugeMem(size_t size);
/* No checking on size.  Memory not initted to 0. */

void *needHugeZeroedMem(long size);
/* Request a large block of memory and zero it. */

void *needMoreMem(void *old, size_t copySize, size_t newSize);
/* Allocate a new buffer, copy old buffer to it, free old buffer. */

void *cloneMem(void *pt, size_t size);
/* Allocate a new buffer of given size, and copy pt to it. */

#define CloneVar(pt) cloneMem(pt, sizeof((pt)[0]))
/* Allocate copy of a structure. */

void *wantMem(size_t size);
/* Want mem just calls malloc - no zeroing of memory, no
 * aborting if request fails. */

void freeMem(void *pt);
/* Free memory will check for null before freeing. */

void freez(void *ppt);
/* Pass address of pointer.  Will free pointer and set it 
 * to NULL. Typical use:
 *     s = needMem(1024);
 *          ...
 *     freez(&s); */

#define AllocVar(pt) (pt = needMem(sizeof(*pt)))
/* Shortcut to allocating a single variable on the heap and
 * assigning pointer to it. */

#define AllocArray(pt, size) (pt = needLargeZeroedMem(sizeof(*pt) * (size)))

#define AllocA(type) needMem(sizeof(type))
/* Shortcut to allocating a variable on heap of a specific type. */

#define AllocN(type,count) ((type*)needLargeZeroedMem(sizeof(type) * (count)))
/* Shortcut to allocating an array on the heap of a specific type. */

#define ExpandArray(array, oldCount, newCount) \
  (array = needMoreMem((array), (oldCount)*sizeof((array)[0]), (newCount)*sizeof((array)[0])))
/* Expand size of dynamically allocated array. */

#define CopyArray(source, dest,count) memcpy(dest,source,(count)*sizeof(dest[0]))
/* Copy count elements of array from source to dest. */

#define CloneArray(a, count) cloneMem(a, (count)*sizeof(a[0]))
/* Make new dynamic array initialized with  count elements of a */

void errAbort(char *format, ...)
/* Abort function, with optional (printf formatted) error message. */
#ifdef __GNUC__
__attribute__ ((noreturn))
#endif
;

void errnoAbort(char *format, ...)
/* Prints error message from UNIX errno first, then does errAbort. */
#ifdef __GNUC__
__attribute__ ((noreturn))
#endif
;

#define internalErr()  errAbort("Internal error %s %d", __FILE__, __LINE__)
/* Generic internal error message */

void warn(char *format, ...);
/* Issue a warning message. */

void zeroBytes(void *vpt, int count);     
/* fill a specified area of memory with zeroes */

#define ZeroVar(v) zeroBytes(v, sizeof(*v))

void reverseBytes(char *bytes, long length);
/* Reverse the order of the bytes. */

void reverseInts(int *a, int length);
/* Reverse the order of the integer array. */

void reverseUnsigned(unsigned *a, int length);
/* Reverse the order of the unsigned array. */

void swapBytes(char *a, char *b, int length);
/* Swap buffers a and b. */

/* Some things to manage simple lists - structures that begin
 * with a pointer to the next element in the list. */
struct slList
    {
    struct slList *next;
    };

int slCount(void *list); 
/* Return # of elements in list.  */

void *slElementFromIx(void *list, int ix);
/* Return the ix'th element in list.  Returns NULL
 * if no such element. */

int slIxFromElement(void *list, void *el);
/* Return index of el in list.  Returns -1 if not on list. */

void slSafeAddHead(void *listPt, void *node); 
/* Add new node to start of list.
 * Usage:
 *    slSafeAddHead(&list, node);
 * where list and nodes are both pointers to structure
 * that begin with a next pointer. 
 */

/* Add new node to start of list, this macro is faster
 * than slSafeAddHead, but has standard macro restriction
 * on what can be safely passed as arguments. */
#define slAddHead(listPt, node) \
    ((node)->next = *(listPt), *(listPt) = (node))

void slAddTail(void *listPt, void *node);
/* Add new node to tail of list.
 * Usage:
 *    slAddTail(&list, node);
 * where list and nodes are both pointers to structure
 * that begin with a next pointer. This is sometimes
 * convenient but relatively slow.  For longer lists
 * it's better to slAddHead, and slReverse when done. 
 */

void *slPopHead(void *listPt);
/* Return head of list and remove it from list. (Fast) */

void *slPopTail(void *listPt);
/* Return tail of list and remove it from list. (Not so fast) */

void *slCat(void *a, void *b);
/* Return concatenation of lists a and b.
 * Example Usage:
 *   struct slName *a = getNames("a");
 *   struct slName *b = getNames("b");
 *   struct slName *ab = slCat(a,b)
 * After this it is no longer safe to use a or b. 
 */

void *slLastEl(void *list);
/* Returns last element in list or NULL if none. */

void slReverse(void *listPt);
/* Reverse order of a list.
 * Usage:
 *    slReverse(&list);
 */

void slSort(void *pList, int (*compare )(const void *elem1,  const void *elem2));
/* Sort a singly linked list with Qsort and a temporary array. 
 * The arguments to the compare function in real, non-void, life
 * are pointers to pointers. */

void slUniqify(void *pList, int (*compare )(const void *elem1,  const void *elem2), void (*free)());
/* Return sorted list with duplicates removed. 
 * Compare should be same type of function as slSort's compare (taking
 * pointers to pointers to elements.  Free should take a simple
 * pointer to dispose of duplicate element, and can be NULL. */

void slRemoveEl(void *pList, void *el);
/* Remove element from singly linked list.  Usage:
 *    slRemove(&list, el);  */

void slFreeList(void *listPt);
/* Free all elements in list and set list pointer to null. 
 * Usage:
 *    slFreeList(&list);
 */

struct slName
/* List of names. The name array is allocated to accommodate full name
 */
    {
    struct slName *next;	/* Next in list. */
    char name[1];               /* Allocated at run time to length of string. */
    };

struct slName *newSlName(char *name);
#define slNameNew newSlName
/* Return a new name. */

int slNameCmp(const void *va, const void *vb);
/* Compare two slNames. */

void slNameSort(struct slName **pList);
/* Sort slName list. */

boolean slNameInList(struct slName *list, char *string);
/* Return true if string is in name list */

void *slNameFind(void *list, char *string);
/* Return first element of slName list (or any other list starting
 * with next/name fields) that matches string. */

char *slNameStore(struct slName **pList, char *string);
/* Put string into list if it's not there already.  
 * Return the version of string stored in list. */

struct slRef
/* Singly linked list of generic references. */
    {
    struct slRef *next;	/* Next in list. */
    void *val;		/* A reference to something. */
    };

struct slRef *refOnList(struct slRef *refList, void *val);
/* Return ref if val is already on list, otherwise NULL. */

void refAdd(struct slRef **pRefList, void *val);
/* Add reference to list. */

void refAddUnique(struct slRef **pRefList, void *val);
/* Add reference to list if not already on list. */

void gentleFree(void *pt);
/* check pointer for NULL before freeing. 
 * (Actually plain old freeMem does that these days.) */

/*******  Some stuff for processing strings. *******/

char *cloneStringZ(char *s, int size);
/* Make a zero terminated copy of string in memory */

char *cloneString(char *s);
/* Make copy of string in dynamic memory */

int differentWord(char *s1, char *s2);
/* strcmp ignoring case - returns zero if strings are
 * the same (ignoring case) otherwise returns difference
 * between first non-matching characters. */

#define sameWord(a,b) (!differentWord(a,b))
/* Return TRUE if two strings are same ignoring case */

#define differentString(a,b) (strcmp(a,b))
/* Returns FALSE if two strings same. */

#define sameString(a,b) (strcmp(a,b)==0)
/* Returns TRUE if two strings same. */

boolean startsWith(char *start,char *string);
/* Returns TRUE if string begins with start. */

#define stringIn(needle, haystack) strstr(haystack, needle)
/* Returns position of needle in haystack or NULL if it's not there. */
/*        char *stringIn(char *needle, char *haystack);      */

boolean endsWith(char *string, char *end);
/* Returns TRUE if string ends with end. */

char lastChar(char *s);
/* Return last character in string. */

boolean wildMatch(char *wildCard, char *string);
/* does a case insensitive wild card match with a string.
 * * matches any string or no character.
 * ? matches any single character.
 * anything else etc must match the character exactly. */

char *memMatch(char *needle, int nLen, char *haystack, int hLen);
/* Returns first place where needle (of nLen chars) matches
 * haystack (of hLen chars) */

void toUpperN(char *s, int n);
/* Convert a section of memory to upper case. */

void toLowerN(char *s, int n);
/* Convert a section of memory to lower case. */

void toggleCase(char *s, int size);
/* toggle upper and lower case chars in string. */

void touppers(char *s);
/* Convert entire string to upper case. */

void tolowers(char *s);
/* Convert entire string to lower case */

char *replaceChars(char *string, char *oldStr, char *newStr);
/*
  Replaces the old with the new.
 The old and new string need not be of equal size
 Can take any lenght string.
 Return value needs to be freeMem'd.
*/

void subChar(char *s, char oldChar, char newChar);
/* Substitute newChar for oldChar throughout string s. */

void stripChar(char *s, char c);
/* Remove all occurences of c from s. */

int countChars(char *s, char c);
/* Return number of characters c in string s. */

int countLeadingChars(char *s, char c);
/* Count number of characters c at start of string. */

int chopString(char *in, char *sep, char *outArray[], int outSize);
/* int chopString(in, sep, outArray, outSize); */
/* This chops up the input string (cannabilizing it)
 * into an array of zero terminated strings in
 * outArray.  It returns the number of strings. 
 * If you pass in NULL for outArray, it will just
 * return the number of strings that it *would*
 * chop. */

extern char crLfChopper[];
extern char whiteSpaceChopper[];
/* Some handy predefined separators. */

unsigned long chopByWhite_ul(char *in, char *outArray[], unsigned long outSize);
/* Like chopString, but specialized for white space separators. */

int chopByWhite(char *in, char *outArray[], int outSize);
/* Like chopString, but specialized for white space separators. */


#define chopLine(line, words) chopByWhite(line, words, ArraySize(words))
#define chopLine_ul(line, words) chopByWhite_ul(line, words, ArraySize(words))
/* Chop line by white space. */

int chopByChar(char *in, char chopper, char *outArray[], int outSize);
/* Chop based on a single character. */

#define chopTabs(string, words) chopByChar(string, '\t', words, ArraySize(words))
/* Chop string by tabs. */

#define chopCommas(string, words) chopByChar(string, ',', words, ArraySize(words))
/* Chop string by commas. */


char *skipLeadingSpaces(char *s);
/* Return first non-white space */

char *skipToSpaces(char *s);
/* Return first white space. */

void eraseTrailingSpaces(char *s);
/* Replace trailing white space with zeroes. */

void eraseWhiteSpace(char *s);
/* Remove white space from a string */

char *trimSpaces(char *s);
/* Remove leading and trailing white space. */

void spaceOut(FILE *f, int count);
/* Put out some spaces to file. */

char *firstWordInLine(char *line);
/* Returns first word in line if any (white space separated).
 * Puts 0 in place of white space after word. */

char *lastWordInLine(char *line);
/* Returns last word in line if any (white space separated).
 * Returns NULL if string is empty.  Removes any terminating white space
 * from line. */

char *nextWord(char **pLine);
/* Return next word in *pLine and advance *pLine to next
 * word. Returns NULL when no more words. */

int stringArrayIx(char *string, char *array[], int arraySize);
/* Return index of string in array or -1 if not there. */

int ptArrayIx(void *pt, void *array, int arraySize);
/* Return index of pt in array or -1 if not there. */

#define stringIx(string, array) stringArrayIx( (string), (array), ArraySize(array))

/* Some stuff that is left out of GNU .h files!? */
#ifndef SEEK_SET
#define SEEK_SET 0
#endif

#ifndef SEEK_CUR
#define SEEK_CUR 1
#endif

#ifndef SEEK_END
#define SEEK_END 2
#endif

void splitPath(char *path, char dir[256], char name[128], char extension[64]);
/* Split a full path into components.  The dir component will include the
 * trailing / if any.  The extension component will include the starting
 * . if any.   Pass in NULL for dir, name, or extension if you don't care about
 * that part. */

char *addSuffix(char *head, char *suffix);
/* Return a needMem'd string containing "headsuffix". Should be free'd
 when finished. */

void chopSuffix(char *s);
/* Remove suffix (last . in string and beyond) if any. */

void chopSuffixAt(char *s, char c);
/* Remove end of string from last occurrence of char c. 
 * chopSuffixAt(s, '.') is equivalent to regular chopSuffix. */

FILE *mustOpen(char *fileName, char *mode);
/* Open a file - or squawk and die. */

void mustWrite(FILE *file, void *buf, size_t size);
/* Write to file or squawk and die. */

#define writeOne(file, var) mustWrite((file), &(var), sizeof(var))
/* Write out one variable to file. */

void mustRead(FILE *file, void *buf, size_t size);
/* Read from a file or squawk and die. */

#define mustReadOne(file, var) mustRead((file), &(var), sizeof(var))
/* Read one variable from file or die. */

#define readOne(file, var) (fread(&(var), sizeof(var), 1, (file)) == 1)
/* Read one variable from file. Returns FALSE if can't do it. */

void writeString(FILE *f, char *s);
/* Write a 255 or less character string to a file.
 * This will write the length of the string in the first
 * byte then the string itself. */

char *readString(FILE *f);
/* Read a string (written with writeString) into
 * memory.  freeMem the result when done. Returns
 * NULL at EOF. */

char *mustReadString(FILE *f);
/* Read a string.  Squawk and die at EOF or if any problem. */

boolean fastReadString(FILE *f, char buf[256]);
/* Read a string into buffer, which must be long enough
 * to hold it.  String is in 'writeString' format. 
 * Returns FALSE at EOF. */
 
void carefulClose(FILE **pFile);
/* Close file if open and null out handle to it. */

boolean carefulCloseWarn(FILE **pFile);
/* Close file if open and null out handle to it. 
 * Return FALSE and print a warning message if there
 * is a problem.*/

char *firstWordInFile(char *fileName, char *wordBuf, int wordBufSize);
/* Read the first word in file into wordBuf. */


int roundingScale(int a, int p, int q);
/* returns rounded a*p/q */

int intAbs(int a);
/* Return integer absolute value */

#define logBase2(x)(log(x)/log(2))
/* return log base two of number */

#define round(a) ((int)((a)+0.5))
/* Round floating point val to nearest integer. */

#ifndef min
#define min(a,b) ( (a) < (b) ? (a) : (b) )
/* Return min of a and b. */
#endif

#ifndef max
#define max(a,b) ( (a) > (b) ? (a) : (b) )
/* Return max of a and b. */
#endif

int  rangeIntersection(int start1, int end1, int start2, int end2);
/* Return amount of bases two ranges intersect over, 0 or negative if no
 * intersection. */

int  positiveRangeIntersection(int start1, int end1, int start2, int end2);
/* Return amount of bases two ranges intersect over, 0 if no
 * intersection. */

bits32 byteSwap32(bits32 a);
/* Swap from intel to sparc order of a 32 bit quantity. */

void removeReturns(char* dest, char* src);
/* Removes the '\r' character from a string.
 * the source and destination strings can be the same, 
 * if there are no threads */
		
int intExp(char *text);
/* Convert text to integer expression and evaluate. 
 * Throws if it finds a non-number. */

double doubleExp(char *text);
/* Convert text to floating point expression and
 * evaluate. */

char* readLine(FILE* fh);
/* Read a line of any size into dynamic memory, return null on EOF */

off_t fileSize(char *fileName);
/* The size of a file. */

boolean fileExists(char *fileName);
/* Does a file exist? */

/*
 Friendly name for strstrNoCase
*/
char *containsStringNoCase(char *haystack, char *needle);

char *strstrNoCase(char *haystack, char *needle);
/* A case-insensitive strstr */

int vasafef(char* buffer, int bufSize, char *format, va_list args);
/* Format string to buffer, vsprintf style, only with buffer overflow
 * checking.  The resulting string is always terminated with zero byte. */

int safef(char* buffer, int bufSize, char *format, ...)
/* Format string to buffer, vsprintf style, only with buffer overflow
 * checking.  The resulting string is always terminated with zero byte. */
#ifdef __GNUC__
__attribute__((format(printf, 3, 4)))
#endif
;

#endif /* COMMON_H */
