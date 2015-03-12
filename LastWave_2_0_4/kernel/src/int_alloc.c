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



#include "lastwave.h"
#include "signals.h"
#include <stdarg.h>

#define MaxNumStrs 200

#ifdef DEBUGALLOC
static int DebugNAllocs[100];
static int DebugNAllocsCopy[100];
static char *DebugTypes[100];
static int DebugNTypes = 0;
char *DebugType;
char *DebugTypeDefault = "Unknown";

void DebugAdd1Alloc() {
  
  int i;
  
  if (DebugNTypes == 0) {
    DebugNAllocs[0] = DebugNAllocsCopy[0];
    DebugNTypes++;
    DebugTypes[0] = DebugTypeDefault;
  }
  
  for (i=0;i<DebugNTypes;i++) {
    if (!strcmp(DebugTypes[i],DebugType)) break;
  }
  
  if (i == DebugNTypes) {
    if (DebugNTypes>=100) {
      Warningf("DEBUG : too many types");
      DebugType = DebugTypeDefault;
      return;
    }
    DebugNTypes++;
    DebugTypes[DebugNTypes-1] = DebugType;
    DebugNAllocs[i] = DebugNAllocsCopy[i] = 0;
  }
  
  DebugNAllocs[i]++;
  
  DebugType = DebugTypeDefault;
}

void DebugRemove1Alloc() {

  int i;
  
  for (i=0;i<DebugNTypes;i++) {
    if (!strcmp(DebugTypes[i],DebugType)) break;
  }

  if (i == DebugNTypes) {
    Warningf("DEBUG : Trying to remove an alloc of type '%s', and there is no such type !",DebugType);
    DebugType = DebugTypeDefault;
    return;
  }

  DebugNAllocs[i]--;
  
  DebugType = DebugTypeDefault;
}

void DebugPrintAlloc() {

  int i;
  int nc,nu;
  int oldn;
 
#ifdef DEBUGALLOCMORE  
  PrintfErr("DEBUG (unchanged) :");   
#endif
  nu = nc = 0;
  oldn = 0;
  for (i=0;i<DebugNTypes;i++) {
    if (DebugNAllocs[i] == DebugNAllocsCopy[i]) {
#ifdef DEBUGALLOCMORE  
      PrintfErr(" %s (%d)",DebugTypes[i],DebugNAllocs[i]);
#endif
      nu+= DebugNAllocs[i];
    } 
    else nc += DebugNAllocs[i];
    oldn += DebugNAllocsCopy[i];
  }
#ifdef DEBUGALLOCMORE  
  PrintfErr("\n");
#endif
  
  if (nc == 0) {
#ifdef DEBUGALLOCMORE  
    PrintfErr("Total alloc : %d\n",nu);
#endif
    return;
  }
  
  PrintfErr("DEBUG (changed)   :");   
  for (i=0;i<DebugNTypes;i++) {
    if (DebugNAllocs[i] != DebugNAllocsCopy[i]) {
      PrintfErr(" %s (%d->%d)",DebugTypes[i],DebugNAllocsCopy[i],DebugNAllocs[i]); 
      DebugNAllocsCopy[i] = DebugNAllocs[i];
    }
  }
  PrintfErr("\nTotal alloc : %d->%d\n",oldn,nc+nu);
}

#endif


/****************************************
 *
 * Basic routines for allocations
 *
 ****************************************/

 
/* Allocation of a certain number of bytes */ 
void *Malloc(size_t size)
{
  void *ptr;
  if (size <= 0) Errorf("*** Serious Error : Malloc() : Can't allocate %d bytes !!",size);
  
  if(ptr = (void *) malloc(size)) {

#ifdef DEBUGALLOC
DebugAdd1Alloc();
#endif
    
    return(ptr);
  }
  else 
    Errorf("*** Serious Error : Malloc() : Can't allocate %d bytes.",size);
}

/* Temporary allocation */
void * TMalloc(size_t size)
{
  void *ptr = Malloc(size);
  TempPtr(ptr);
  return(ptr);
}


/* ReAllocation of a certain number of bytes */ 
void *Realloc(void *ptr, size_t size)
{
  void *ptr1;
  if (size <= 0) Errorf("*** Serious Error : Realloc() : Can't allocate %d bytes !!",size);
  
  if(ptr1 = (void *) realloc(ptr,size)) {
    return(ptr1);
  }
  else Errorf("*** Serious Error : Realloc() : Can't reallocate %d bytes.",size);
}

/* Allocation of a certain number of bytes and initialization */ 
void *Calloc(int n, size_t size)
{
  void *ptr;
  if (size <= 0) Errorf("*** Serious Error : Calloc() : Can't allocate %d bytes !!",size);
  
  if(ptr = (void *) calloc(n,size)) {

#ifdef DEBUGALLOC
DebugAdd1Alloc();
#endif
    return(ptr);
  }
  else Errorf("*** Serious Error : Calloc() : Can't allocate %d bytes.",size);
}


/* allocation of an array of chars  */
char *CharAlloc(int size)
{
  char *ptr;
  
  if (size <= 0) Errorf("*** Serious Error : CharAlloc() : Can't allocate %d chars !!",size);
  
  if(ptr = (char *) malloc(sizeof(char)*size)){

#ifdef DEBUGALLOC
DebugAdd1Alloc();
#endif

    return(ptr);
  }
  else Errorf("*** Serious Error : CharAlloc() : Can't allocate %d chars !!",size);
}

/* allocation of an array of floats */
float *FloatAlloc(int size)
{
  float *ptr;
  
  if (size <= 0) Errorf("*** Serious Error : FloatAlloc() : Can't allocate %d floats !!",size);
  
  if(ptr = (float *) malloc(sizeof(float)*size)) {

#ifdef DEBUGALLOC
DebugAdd1Alloc();
#endif

    return(ptr);
  }
  else Errorf("*** Serious Error : FloatAlloc() : Can't allocate %d floats !!",size);
}

/* allocation of an array of floats */
float *FloatCAlloc(int size)
{
  float *ptr;
  
  if (size <= 0) Errorf("*** Serious Error : FloatCAlloc() : Can't allocate %d floats !!",size);
  
  if(ptr = (float *) calloc(sizeof(float),size)) {

#ifdef DEBUGALLOC
DebugAdd1Alloc();
#endif

    return(ptr);
  }
  else Errorf("*** Serious Error : FloatCAlloc() : Can't allocate %d floats !!",size);
}


/* allocation of an array of doubles */
double *DoubleAlloc(int size)
{
  double *ptr;
  
  if (size <= 0) Errorf("*** Serious Error : DoubleAlloc() : Can't allocate %d floats !!",size);
  
  if(ptr = (double *) malloc(sizeof(double)*size)) {

#ifdef DEBUGALLOC
DebugAdd1Alloc();
#endif

    return(ptr);
  }
  else Errorf("*** Serious Error : DoubleAlloc() : Can't allocate %d doubles !!",size);
}


/* allocation of an array of ints */
int *IntAlloc(int size)
{
  int *ptr;
  
  if (size <= 0) 
    Errorf("*** Serious Error : IntAlloc() : Can't allocate %d ints !!",size);
  
  if(ptr = (int *) malloc(sizeof(int)*size)) {

#ifdef DEBUGALLOC
DebugAdd1Alloc();
#endif

    return(ptr);

  }
  else Errorf("*** Serious Error : IntAlloc() : Can't allocate %d ints !!",size);
}

void Free(void * ptr)
{
#ifdef DEBUGALLOC
DebugRemove1Alloc();
#endif

  free(ptr);
 }
 
/**************************************************************************
 *
 * Functions managing the temporary allocation
 *    (it uses a lot the fields of the current toplevel).
 *
 **************************************************************************/

/* Make a Pointer temporary */
void TempPtr(void *ptr)
{
  if (ptr == NULL) 
    Errorf("TempPtr() (weird bug) : ptr is NULL");
  
  if (toplevelCur->nTempAlloc == MaxNumTempAllocs)  Errorf("TempPtr() : Temporary memory too small");
    
  toplevelCur->tempAlloc[toplevelCur->nTempAlloc] = ptr;
  toplevelCur->nTempAlloc++;
}


/* 
 * Make a VALUE temporary : 
 *   To distinguish them from regular pointers, we just add a NULL ptr rigth after
 */

 
void TempValue_(VALUE value)
{
  if (value == NULL) Errorf("TempValue() (weired bug) : value is NULL");
  
  if (toplevelCur->nTempAlloc+1 >= MaxNumTempAllocs)  Errorf("TempValue() : Temporary memory too small");
    
  toplevelCur->tempAlloc[toplevelCur->nTempAlloc] = value;
  toplevelCur->nTempAlloc++;

  toplevelCur->tempAlloc[toplevelCur->nTempAlloc] = NULL;
  toplevelCur->nTempAlloc++;
}


/* We set a temporary zone of memory */
void SetTempAlloc(void)
{
  if (toplevelCur->nMarkerTempAlloc == MaxNumMarkerTempAllocs) 
    Errorf("SetTempAlloc() : Number of imbricated temporary markers is too high");
    
  toplevelCur->markerTempAlloc[toplevelCur->nMarkerTempAlloc] = toplevelCur->nTempAlloc;
  toplevelCur->nMarkerTempAlloc++;
/*  Printf(">> IN  %d \n",toplevelCur->nMarkerTempAlloc); */
}



/* The command for debugging allocation */
int flagOn = 0;
void C_DebugAlloc(char **argv)
{
  flagOn = 1-flagOn;
}

/* We free the last temporary zone of memory */
void ClearTempAlloc(void)
{
  unsigned *marker = toplevelCur->markerTempAlloc;
  void** ptr = toplevelCur->tempAlloc;
  unsigned nMarker = toplevelCur->nMarkerTempAlloc;
  unsigned nPtr = toplevelCur->nTempAlloc;
  VALUE value;
  int i = 0;
  
  
  while (marker[nMarker-1] != nPtr) {
    i++;
    
    /* Case we must delete a VALUE */
    if (ptr[nPtr-1] == NULL) {
      nPtr--;
      value = (VALUE) ptr[nPtr-1];
      if (flagOn) Printf("Remove Temp value %p [%d]\n",value,value->nRef-1);
      DeleteValue(value);
      nPtr--;
    } 
    
    /* Case of a regular pointer */
    else { 
      Free(ptr[nPtr-1]);
      nPtr--;
    }
  }

/*  Printf("<< OUT %d %d\n",toplevelCur->nMarkerTempAlloc,i); */

  toplevelCur->nMarkerTempAlloc--;  
  toplevelCur->nTempAlloc = nPtr;  
}


/* We free all the temporary zones of memory of the current toplevel */
void ClearAllTempAlloc(void)
{
  void** ptr = toplevelCur->tempAlloc;
  unsigned short nPtr = toplevelCur->nTempAlloc;
  VALUE value;
  
  while (nPtr != 0) {
  
    /* Case we must delete a VALUE */
    if (ptr[nPtr-1] == NULL) {
      nPtr--;
      value = (VALUE) ptr[nPtr-1];
      DeleteValue(value);
      nPtr--;
      if (flagOn) Printf("Remove Temp value %p\n",value);
    } 
    
    else {
      Free(ptr[nPtr-1]);
      nPtr--;
    }
  }

  toplevelCur->nMarkerTempAlloc = 0;  
  toplevelCur->nTempAlloc = 0;  


#ifdef DEBUGALLOC
DebugPrintAlloc();
#endif

}








