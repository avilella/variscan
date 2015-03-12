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



/********************************************************************/
/*                                                                  */
/*   window_event.h :                     */
/*                                                                  */
/********************************************************************/


/*
 * The event types 
 */


#define NoEvent 0

#define ButtonDown (1l<<0)
#define ButtonUp (1l<<1)

#define KeyDown (1l<<2)
#define KeyUp (1l<<3)

#define Enter (1l<<4)
#define Leave (1l<<5)  
#define Draw (1l<<6)
#define Del (1l<<7)
#define Resize (1l<<8)
  
#define MouseMotion (1l<<9)
  
#define ErrorEvent (1l<<10)
#define TimeEvent (1l<<11)
#define DelayEvent (1l<<12)

/*
 * Each EventCategory correspond to a set of event types.
 */
 
enum { 
  ButtonEventCategory = 0,
  KeyEventCategory,
  EnterLeaveEventCategory,
  MotionEventCategory,
  OtherEventCategory,
  LastEventCategory
};

/*
 * Warning : The number of event categories must be used for the size
 * of the 'theBindings' array of a GCLASS
 */

/*
 * The different buttons (these definitions are used both as values and as masks) 
 */
 
#define LeftButton (1l<<0)
#define MiddleButton (1l<<1)
#define RightButton (1l<<2)
#define NoButton (1l<<3)

#define ButtonMask 0x000F

/*
 * The different modifier keys (these definitions are used both as values and as masks)
 */
#define ModShift (1l<<10)
#define ModCtrl (1l<<11)
#define ModOpt (1l<<12)
#define ModAny (1l<<13)

#define ModMask 0xFC00

/*
 * The different key codes of 'special keys'
 *
 *   WARNING : If you add one key code don't forget to add its name in the SpecialKeyNames array
 *             in window_event.s
 *
 */

#define KeyMask 0x03FF

#define EscapeKC 27              /* For xterm compatibilities we choose the ascii code */
#define NewlineKC ((int) '\n')   /* For xterm compatibilities we choose the ascii code */

#define EofKC 256     /* A key code that will nether conflict with the other ones */

#define FirstKC 0x0101

enum {

 RightKC = FirstKC,
 UpKC,LeftKC,DownKC,
 
 HomeKC,EndKC,ClearKC,
 
 DeleteKC,TabKC,
 
 F1KC,F2KC,F3KC,F4KC,F5KC,F6KC,F7KC,F8KC,F9KC,F10KC,F11KC,F12KC,F13KC,F14KC,F15KC,
 
 AnyKC,
 
 LastKC
 
};

/*
 * Function for converting any key code into a string 
 */
extern char *KeyCode2Str(unsigned long key, char flagBraces);
   
/*
 * Function for getting values of @variables
 */
 extern void *GetEventVariable(char *name,char **);

   
/*
 * The event structure 
 */
typedef struct event {

  /* the type of the event */  
  unsigned long type;
  
  /* The object it happened (or NULL if in terminal) */
  GOBJECT object;
    
  /* The button that was (un)pressed (along with the modifiers) */
  unsigned long button;
      
  /* The key that was (un)pressed (along with the modifiers) */
  unsigned long key;

  /* local coordinates */
  float x,y,w,h;

  /* global coordinates */
  int i,j,m,n;
  
} *EVENT;


/*
 * The binding structure 
 */
typedef struct binding {

  /* The event type it is binded too */
  unsigned long eventType;

  /* The buttons it is binded too in case a button is pressed (this is a mask) (along with the modifiers) */
  int button;
  
  /* The key (and modifier) sequence that it is binded too in case of a KeyUp or KeyDown event */
  unsigned long *keys;
  int nKeys;
  
  /* Used for time and delay events */
  float time,delay;
  
  /* The corresponding script to execute */
  SCRIPT script;
  
  /* A pointer to the group of bindings it belongs to (NULL if none) */  
  struct bindingGroup *group;

  /* Pointers to the next and the previous bindings in the chained list */
  struct binding *next,*previous;
  
  /* Pointer to the chained list it belongs to */
  struct binding **chainedList;
  
  /* The state of the binding could be either one of the values below */
  char state; 
  
} *BINDING;

/*
 * State values for bindings 
 *   BindingOff : the associated script is not being executed
 *   BindingOn  : the associated script is being executed
 *   Binding2BeDeleted  : the associated script is being executed ans asks that the binding 
 *                        should be deleted. Thus it will be deleted at the end of the script.
 */
enum {
  BindingOff = 0,
  BindingOn,
  Binding2BeDeleted
};
  

/*
 * The binding group structure
 */
typedef struct bindingGroup {

  /* The name of the group */
  char *name;
  
  /* Pointers to the next and the previous group bindings in the chained list */
  struct bindingGroup *next,*previous;

  /* Pointer to the chained list it belongs to */
  struct bindingGroup **chainedList;
 
  /* Is this group active ? */
  char flagActive;
  
  /* a Useful flag */
  char flag;
  
  /* A one-line Help */
  char *help;
  
  /* Number of bindings attached to it */
  int nBindings;
   
} *BINDINGGROUP;
 

/*
 * Function for initializing the binding structures 
 */  
extern void InitBindings(void);  
 
extern void SendEvent(EVENT event);



