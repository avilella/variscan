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
/*   event.c   Deals with window manager events                             */
/*                                                                          */
/****************************************************************************/



#include "lastwave.h"

/* In INT_MAIN.c */
extern int TheLoop (void);  


/*
 * The name for the special keys
 *
 *   WARNING : If you add one special key code in window_event.h don't forget to add its name here
 */
 
char SpecialKeyNames[][10] = {
  
  "right","up","left","down",
  
  "home","end","clear",
  
  "delete","tab",
  
  "f1","f2","f3","f4","f5","f6","f7","f8","f9","f10","f11","f12","f13","f14","f15",
  
  "any"
};



/**************************************************************
 *
 *    Miscellaneous Functions 
 *
 **************************************************************/

/*
 * Conversion of a key code to a printable string
 *   It Prints something like "{Ctrl up}" or simply "a".
 *   If flagBraces == YES then it adds some braces if the string is longer than just 1 char. 
 */
 
char *KeyCode2Str(unsigned long key, char flagBraces)
{
  static char result[100];
  char str[100],str1[2];
  unsigned long k = key & KeyMask;
  unsigned long m = key & ModMask;
  
  str[0] = '\0';
  
  if (key == EOF) strcat(str,"eof"); 
  else {  
    if (m == ModShift) strcat(str,"shift ");
    else if (m == ModCtrl) strcat(str,"ctrl ");
    else if (m == ModOpt) strcat(str,"opt ");
    else if (m == ModOpt + ModCtrl) strcat(str,"ctrlOpt ");
    else if (m == ModShift + ModCtrl) strcat(str,"ctrlShift ");
    else if (m == ModShift+ModOpt) strcat(str,"optShift ");
    else if (m == ModShift+ModOpt+ModCtrl) strcat(str,"ctrlOptShift ");
  
    if (k == '\r' || k == '\n') strcat(str,"\n");
    else if (k<' ' ||  k >'~') {
      if (k == EscapeKC) strcat(str,"esc");
      else if (k == EofKC) strcat(str,"eof");
      else if (k >= FirstKC && k < LastKC) strcat(str,SpecialKeyNames[k-FirstKC]);
      else {
       sprintf(result,"0x%lx",k);
       strcat(str,result);
     }
    }
    else {
      str1[0] = k; 
      str1[1] = '\0';
      strcat(str,str1);
    }
  }
  
  if (flagBraces == YES && strlen(str) != 1) {
    strcpy(result,"{");
    strcat(result,str);
    strcat(result,"}");
  }
  else strcpy(result,str);
  
  return(result);
}

/*
 * Conversion of a button field to a printable string
 *   It Prints something like "Ctrl left" or simply "left".
 */
 
char *ButtonCode2Str(unsigned long button)
{
  static char str[30];
  unsigned long k = button & ButtonMask;
  unsigned long m = button & ModMask;
  
  str[0] = '\0';
  
  if (m == ModShift) strcat(str,"shift ");
  else if (m == ModCtrl) strcat(str,"ctrl ");
  else if (m == ModOpt) strcat(str,"opt ");
  else if (m == ModOpt + ModCtrl) strcat(str,"ctrlOpt ");
  else if (m == ModShift + ModCtrl) strcat(str,"ctrlShift ");
  else if (m == ModShift+ModOpt) strcat(str,"optShift ");
  else if (m == ModShift+ModOpt+ModCtrl) strcat(str,"ctrlOptShift ");

  if (k == LeftButton) strcat(str,"left");  
  else if (k == RightButton) strcat(str,"right");  
  else if (k == MiddleButton) strcat(str,"middle");
  else strcat(str,"no");
  
  return(str);  
}


/*
 * Function that associate a combination of modifiers to a string (returns 0 if failed)
 */
static unsigned long Str2Modifiers(char *mod)
{
  if (mod != NULL) {
    if (!strcmp(mod,"shift")) return(ModShift);
    if (!strcmp(mod,"ctrl")) return(ModCtrl);
    if (!strcmp(mod,"ctrlShift")) return(ModCtrl+ModShift);
    if (!strcmp(mod,"opt")) return(ModOpt);
    if (!strcmp(mod,"ctrlOpt")) return(ModCtrl+ModOpt);
    if (!strcmp(mod,"optShift")) return(ModOpt+ModShift);
    if (!strcmp(mod,"ctrlOptShift")) return(ModCtrl+ModOpt+ModShift);
    if (!strcmp(mod,"any")) return(ModAny);
  }
  return(0);
}
    

/************************************************************************
 *
 *  Miscellaneous functions on Bindings and binding groups 
 *
 ************************************************************************/

/*
 * The maximum length of binded key sequences  
 */
# define MaxLengthKeySeq 40

/*
 * The chained list of the different group names
 */
static BINDINGGROUP theBindingGroups;

/*
 * The chained lists of the terminal bindings
 */
static BINDING theTerminalBindings[LastEventCategory];


/*
 * Initialization of the terminal binding structures 
 */
void InitTerminalBindings(void)
{
  int i;
  
  for (i=0;i<LastEventCategory;i++) theTerminalBindings[i] = NULL;
}

/*
 * Delete a binding group
 */
static void DeleteBindingGroup(BINDINGGROUP g)
{
  g->nBindings--;
  if (g->nBindings != 0) return;
  
  if (g->previous != NULL) {
    if (g->next != NULL) g->next->previous = g->previous;
    g->previous->next = g->next;
  }
  else {
    if (g->next != NULL) g->next->previous = NULL;
    *(g->chainedList) = g->next;
  }
  
  DeleteStr(g->name);
  
  if (g->help != NULL) DeleteStr(g->help);

  Free(g);
}

/*
 * Creates a binding group (or returns it if it already exists 
 */
static BINDINGGROUP NewBindingGroup(char *name, char *help)
{  
  BINDINGGROUP group;
  
  /* Looking for the group name. if it exists and creates it if it does not*/
  group = theBindingGroups;
  while (group != NULL) {
    if (!strcmp(group->name,name)) break;
    group = group->next;
  }
  if (group == NULL) {
    group = (BINDINGGROUP) Malloc(sizeof (struct bindingGroup));
    group->name = CopyStr(name);
    group->flagActive = NO;
    group->previous = NULL;
    group->next = theBindingGroups;
    group->chainedList = &(theBindingGroups);
    group->nBindings = 0;
    group->help = NULL;
    if (group->next != NULL) group->next->previous = group;
    theBindingGroups = group;
  }

  if (group->help != NULL && help != NULL) {
    Free(group->help);
    group->help = NULL;
  }
  
  if (help != NULL) group->help = CopyStr(help);
  
  return(group);
}

/*
 * Delete a binding 
 */
static void DeleteBinding(BINDING b)
{
  DeleteBindingGroup(b->group);
  
  if (b->previous != NULL) {
    if (b->next != NULL) b->next->previous = b->previous;
    b->previous->next = b->next;
  }
  else {
    if (b->next != NULL) b->next->previous = NULL;
    *(b->chainedList) = b->next;
  }
  
  DeleteScript(b->script);
  
  if (b->keys != NULL) Free(b->keys);
  
  Free(b);
}


/*
 * Get the corresponding 'category' of an eventType
 */
 
static int GetBindingCategory(unsigned long eventType)
{
  if (eventType & ButtonDown || eventType & ButtonUp) return(ButtonEventCategory);
  if (eventType & KeyDown || eventType & KeyUp) return(KeyEventCategory);
  if (eventType & Enter || eventType & Leave) return(EnterLeaveEventCategory);
  if (eventType & MouseMotion) return(MotionEventCategory);
  if (eventType & Draw) return(OtherEventCategory);
  if (eventType & Del) return(OtherEventCategory);
  if (eventType & Resize) return(OtherEventCategory);
  if (eventType & ErrorEvent) return(OtherEventCategory);
  if (eventType & TimeEvent) return(OtherEventCategory);
  if (eventType & DelayEvent) return(OtherEventCategory);
  Errorf("GetBindingCategory() : Unknown event type '%d'",eventType);
}


/************************************************************************
 *
 *  Main command for creating a new binding
 *
 ************************************************************************/


void C_SetBinding(char **argv)
{
  GCLASS class;
  char *type;
  char lookForModifiers;
  char *groupName;
  char *help;
  char *endp;
  long lval;
  struct binding b;
  char **keyList,**list;
  unsigned long keys[MaxLengthKeySeq];
  int nKeys;
  char *keyName;
  BINDING binding;
  BINDING *theBindings;
  BINDINGGROUP group;
  int cat,i;
  float time;
  
  /* First we try to read a group name and a one line help */  
  argv = ParseArgv(argv,tSTR,&groupName,-1);
  
  /* If there is just one argument left then we process ! */
  if (argv[1] == NULL) {
    argv = ParseArgv(argv,tSTR,&help,0);
    NewBindingGroup(groupName,help);
    return;
  }
  
  /* Try to read the class */
  argv = ParseArgv(argv,tWORD,&help,-1);
  if (!strcmp("terminal",help)) theBindings = theTerminalBindings;
  else if ((class = (GCLASS) GetElemHashTable(theGClasses,help)) != NULL) theBindings = class->theBindings;
  else  Errorf("Unknown '%s' gclass",help);

    
  /* Initialize the binding fields and some related variables */
  b.button = NoButton;
  b.keys = NULL;
  b.script = NULL;
  nKeys = 0;
  lookForModifiers = YES;
  
  /* Get the event type */
  argv = ParseArgv(argv,tWORD,&type,-1);

  /*
   * Set the 'eventType', 'button', 'modifiers', 'keys', 'keyModifiers' binding fields
   */
     
  /* Buttons events */
  if (!strcmp(type,"leftButtonUp")) {b.eventType = ButtonUp; b.button = LeftButton;}
  else if (!strcmp(type,"leftButtonDown")) {b.eventType = ButtonDown; b.button = LeftButton;}
  else if (!strcmp(type,"leftButton")) {b.eventType = ButtonDown+ButtonUp; b.button = LeftButton;}
  else if (!strcmp(type,"leftButtonMotion")) {b.eventType = MouseMotion; b.button = LeftButton;}
  else if (!strcmp(type,"middleButtonUp")) {b.eventType = ButtonUp; b.button = MiddleButton;}
  else if (!strcmp(type,"middleButtonDown")) {b.eventType = ButtonDown; b.button = MiddleButton;}
  else if (!strcmp(type,"middleButton")) {b.eventType = ButtonDown+ButtonUp; b.button = MiddleButton;}
  else if (!strcmp(type,"middleButtonMotion")) {b.eventType = MouseMotion; b.button = MiddleButton;}
  else if (!strcmp(type,"rightButtonUp")) {b.eventType = ButtonUp; b.button = RightButton;}
  else if (!strcmp(type,"rightButtonDown")) {b.eventType = ButtonDown; b.button = RightButton;}
  else if (!strcmp(type,"rightButton")) {b.eventType = ButtonDown+ButtonUp; b.button = RightButton;}
  else if (!strcmp(type,"rightButtonMotion")) {b.eventType = MouseMotion; b.button = RightButton;}
  else if (!strcmp(type,"buttonUp")) {b.eventType = ButtonUp; b.button = LeftButton + MiddleButton + RightButton;}
  else if (!strcmp(type,"buttonDown")) {b.eventType = ButtonDown; b.button =  LeftButton + MiddleButton + RightButton;}
  else if (!strcmp(type,"buttonMotion")) {b.eventType = MouseMotion; b.button = LeftButton + MiddleButton + RightButton;}
  
  /* Motion/Enter/leave events */
  else if (!strcmp(type,"motion")) {b.eventType = MouseMotion;b.button = NoButton;}
  else if (!strcmp(type,"leave")) {lookForModifiers = NO;b.eventType = Leave;}
  else if (!strcmp(type,"enter")) {lookForModifiers = NO;b.eventType = Enter;}
  else if (!strcmp(type,"enterLeave")) {lookForModifiers = NO;b.eventType = Enter+Leave;}
  
  /* Key Events */
  else if (!strncmp(type,"key",3)) {
  
    /* Reading the list of keys */
    argv = ParseArgv(argv,tLIST,&keyList,-1);
    
    /* Loop on this list */
    while (*keyList != NULL) {

      if (nKeys >= MaxLengthKeySeq) Errorf("Binded key sequence is too long (should be <= %d)",MaxLengthKeySeq);
      
      /* Parse the key name and the eventual modifier keys */
      ParseWordList(*keyList,&list);
      keyList++;
      if (*list == NULL) Errorf("Missing key name for key binding");
      if (*(list+1) == NULL) {
        keyName = *list;
        keys[nKeys] = 0;
      }
      else if (*(list+2) == NULL) {
        keys[nKeys] = Str2Modifiers(*list);
        if (keys[nKeys] == 0) Errorf("Bad modifier '%s' in key binding",*list);
        keyName = *(list+1);
      }
      else Errorf("Too long key name for key binding");

      /* Get the key code */
      if (keyName[1] == '\0') keys[nKeys++] += *keyName;
      else if (keyName[0] == '0' && keyName[1] == 'x') {
        lval = strtol(keyName,&endp,0);
	    if (*endp != '\0' || lval < 0 || lval >= 256) Errorf("Bad key code '%s'",keyName);
	    keys[nKeys++] += lval;
      }
      else if (!strcmp(keyName,"esc")) keys[nKeys++] += EscapeKC;
      else if (!strcmp(keyName,"newline")) keys[nKeys++] += NewlineKC;
      else {
        for (i=FirstKC;i<LastKC;i++) {
          if (!strcmp(keyName,SpecialKeyNames[i-FirstKC])) {
            keys[nKeys++] += i;
            break;
          }
        }
        if (i == LastKC) Errorf("Unknown key name '%s'",keyName);
      }
      keys[nKeys] = 0;
    }
    
    if (nKeys == 0) Errorf("Missing key name for key binding");
    
    if (!strcmp(type,"keyUp")) {b.eventType = KeyUp;}
    else if (!strcmp(type,"keyDown")) {b.eventType = KeyDown;}
    else if (!strcmp(type,"key")) {b.eventType = KeyDown + KeyUp;}
    else Errorf("Bad event type '%s'",type);
    
    if ((b.eventType & KeyUp) && nKeys >1) Errorf("Cannot bind KeyUp sequences (You should use the keyword KeyDown)");
    
    lookForModifiers = NO;
  }

  /* Draw Events */
  else if (!strcmp(type,"draw")) {
    lookForModifiers = NO;
    b.eventType = Draw;
  }

  /* Delete events */
  else if (!strcmp(type,"delete")) {
    lookForModifiers = NO;
    b.eventType = Del;
  }
  
  /* Error Events */
  else if (!strcmp(type,"error")) {
    lookForModifiers = NO;
    b.eventType = ErrorEvent;
  }

  /* Delay Events */
  else if (!strcmp(type,"delay")) {
    if (theBindings != theTerminalBindings) Errorf("Delay bindings must be associated to the terminal");
    lookForModifiers = NO;
    b.eventType = DelayEvent;
    b.time = MyTime();
    argv = ParseArgv(argv,tFLOAT,&time,-1);
    if (time <0) Errorf("The delay should be a positive float");
    b.delay = time;
  }

  /* Time Events */
  else if (!strcmp(type,"time")) {
    if (theBindings != theTerminalBindings) Errorf("Delay bindings must be associated to the terminal");
    lookForModifiers = NO;
    b.eventType = TimeEvent;
    argv = ParseArgv(argv,tFLOAT,&time,-1);
    if (time <0) Errorf("The time should be a positive float");
    b.time = time;
  }

  /* Unknow event type */      
  else Errorf("Bad event type '%s'",type);
  
  
  /* Looking for modifiers if necessary (This is done only for button or motion events) */
  if (lookForModifiers) {
    argv = ParseArgv(argv,tWORD_,NULL,&keyName,-1);
    if (keyName != NULL) {
      b.button += Str2Modifiers(keyName);
      if ((ModMask & b.button) == 0) argv--;
    }
  }

  /* Now get the script */
  argv = ParseArgv(argv,tSCRIPT,&(b.script),-1);
    
  /* That's it */  
  NoMoreArgs(argv);


 /*
  * So let's create the binding ! 
  */
  
  /* Looking for the group name if it exists and creates it if it does not*/
  group = NewBindingGroup(groupName,NULL);

  /* Allocation of the binding */ 
  cat = GetBindingCategory(b.eventType);
  binding = (BINDING) Malloc(sizeof(struct binding));
  binding->group = group;
  group->nBindings++;
  binding->previous = NULL;
  binding->next = theBindings[cat];
  binding->chainedList = &(theBindings[cat]);
  if (binding->next != NULL) binding->next->previous = binding;  
  theBindings[cat] = binding;
  
  /* Set the fields of the newly created binding */
  binding->eventType = b.eventType;
  binding->button = b.button;
  binding->time = b.time;
  binding->delay = b.delay;
  binding->script = b.script;
  b.script->nRef++;
  binding->state = BindingOff;
  if (nKeys == 0) {binding->keys = NULL; binding->nKeys = 0;}
  else {
    binding->keys = (unsigned long *) Malloc(sizeof(unsigned long)*(nKeys+1));
    binding->nKeys = nKeys;
    for (i=0;i<=nKeys;i++) binding->keys[i] = keys[i];
  }
}



/************************************************************************
 *
 *  Main command for for editing binding groups
 *
 ************************************************************************/
 
void C_Binding(char **argv)
{
  char *command,*groupName;
  char flagActivate,flagDeactivate,flagDelete,flagInfo;
  BINDINGGROUP g;
  BINDING *theBindings;
  char *str;
  BINDING b,b1;
  int w,r;
  AHASHELEM e; 
  GCLASS class;
  LISTV lv,lv1;
    
  /* Read the command and the group name */
  argv = ParseArgv(argv,tWORD,&command,tSTR,&groupName,tWORD_,NULL,&str,0);
  
  /* Reading a class name */ 
  if (str != NULL) {
    class = (GCLASS) GetElemHashTable(theGClasses,str);
    if (class == NULL) Errorf("Bad class name '%s'",str);
  }
  else class = NULL;
    
  /* Set the command flags */
  flagActivate = flagDeactivate = flagDelete = flagInfo = NO;
  if (!strcmp(command,"activate")) flagActivate = YES;
  else if (!strcmp(command,"deactivate")) flagDeactivate = YES;
  else if (!strcmp(command,"delete")) flagDelete = YES;
  else if (!strcmp(command,"info")) flagInfo = YES;
  else Errorf("Bad action %s",command);
  
  
  /* Init the binding group flags if necessary */
  if (flagInfo) for (g = theBindingGroups;g!= NULL; g = g->next) g->flag = 0;
    
  /*
   * First we take care of the case where we need to perform a loop on the bindings (not on the binding groups)
   */
  
  if (flagDelete) {
    if (class == NULL) {
      theBindings = theTerminalBindings;
      for (w=0;w<LastEventCategory;w++) {
        for (b = theBindings[w]; b != NULL ;) {
          if (MatchStr(b->group->name,groupName)) {
            b1 = b;
            b = b->next;
            if (b1->state == BindingOn) b1->state = Binding2BeDeleted;
            else DeleteBinding(b1);
          }
          else b = b->next;
        }
      }
    }
    for (r = 0; r<theGClasses->nRows;r++) {
      for (e = theGClasses->rows[r]; e != NULL; e = e->next) { 
        if (class != NULL && class != ((GCLASS) e)) continue;
        theBindings = ((GCLASS) e)->theBindings;
        for (w=0;w<LastEventCategory;w++) {
          for (b = theBindings[w]; b != NULL ;) {
            if (MatchStr(b->group->name,groupName)) {
              b1 = b;
              b = b->next;
              if (b1->state == BindingOn) b1->state = Binding2BeDeleted;
              else DeleteBinding(b1);
            }
            else b = b->next;
          }
        }
      }
    }
    return;
  }
       
  
  /*
   * Then we take care of the (simpler) case where we need to perform a loop on the binding groups
   */
  if (flagInfo) {
    lv = TNewListv(); 
    SetResultValue(lv);
  }
  for (g = theBindingGroups;g!= NULL; g = g->next) {
    if (MatchStr(g->name,groupName)) { 
      if (class) {
        theBindings = class->theBindings;
        for (w=0;w<LastEventCategory;w++) {
          for (b = theBindings[w]; b != NULL ;b = b->next) {
            if (b->group == g) break;
          }
          if (b != NULL) break;
        }
        if (w == LastEventCategory) continue;
      }
      if (flagActivate) g->flagActive = YES;
      else if (flagDeactivate) g->flagActive = NO;
      else if (flagInfo) {
        if (g->flag == 0) {
          lv1 = TNewListv();
          AppendStr2Listv(lv1,g->name);
          AppendInt2Listv(lv1,g->flagActive);
          if (g->help != NULL)  AppendStr2Listv(lv1,g->help);
          AppendValue2Listv(lv,(VALUE) lv1);
          g->flag = 1;
        }
      }
   }
  }
}


/************************************************************************
 *
 *  Functions that manage event variable dispatching
 *
 ************************************************************************/


/*
 * Function to get an event variable (which is a field of a stored event)
 *   
 *   name : the name of the event variable
 *   type : a pointer to the expected type of the variable (strType, signalType...)
 *          or a pointer to NULL if any type is accepted
 *
 *   --> returns the pointer to the content of the variable
 *       amd the type of this content if *type == NULL.
 */
 
void *GetEventVariable(char *name,char **type) /* ?????????????? a jeter */
{
  static char result[100];

  EVENT event = toplevelCur->lastEvent;
  
  /* If no event was stored --> NULL */
  if (event == NULL) return(NULL);
  if (event->type == NoEvent) return(NULL);
  
  /* A simple test */
  if (type == NULL) Errorf("GetEventVariable() : type should not be NULL");
  
  /*
   * We test all the  @variables 
   */
      
  if (*type == strType || *type == NULL) {
      
    /* Key variable */  
    if (!strcmp("key",name)) {
      *type = strType;
      if (event->type == KeyDown || event->type == KeyUp) return(KeyCode2Str(event->key,NO));
      else return(NULL);
    }

    /* Window variable */  
    if (!strcmp("window",name)) {
      *type = strType;
      if (event->object != NULL) return(GetWin(event->object)->name);
      else {
        strcpy(result,"");
        return((void *) result);
      }
    }

    /* Objname variable */  
    if (!strcmp("objname",name)) {
      *type = strType;
      if (event->object != NULL) return(GetNameGObject(event->object));
      else {
        strcpy(result,"");
        return((void *) result);
      }
    }     
  
      
    /* Type variable */
    if (!strcmp("type",name)) {
      *type = strType;
      if (event->type == KeyDown) strcpy(result,"keyDown");
      else if (event->type == KeyUp) strcpy(result,"keyUp");
      else if (event->type == ButtonUp) strcpy(result,"buttonUp");
      else if (event->type == ButtonDown) strcpy(result,"buttonDown");
      else if (event->type == Enter) strcpy(result,"enter");
      else if (event->type == Leave) strcpy(result,"leave");
      else if (event->type == MouseMotion) strcpy(result,"mouseMotion");
      else if (event->type == Draw) strcpy(result,"draw");
      else if (event->type == Del) strcpy(result,"delete");
      else if (event->type == ErrorEvent) strcpy(result,"errorEvent");
      else Errorf("GetEventVariable() : event type %d unknown",event->type);
      return((void *) result);
    }

    /* Button variable */
    if (!strcmp("button",name)) {
      *type = strType;
      if (event->type == ButtonDown || event->type == ButtonUp || event->type == MouseMotion) 
        return(ButtonCode2Str(event->button));
      else return(NULL);
    }
  
    /* i, j, m and n variables (mouse global coordinates) */
    if (!strcmp("i",name)){
      *type = strType;
      sprintf(result,"%d",event->i);
      return((void *) result);
    }
    if (!strcmp("j",name)){
      *type = strType;
      sprintf(result,"%d",event->j);
      return((void *) result);
    }
    if (!strcmp("m",name)){
      *type = strType;
      sprintf(result,"%d",event->m);
      return((void *) result);
    }
    if (!strcmp("n",name)){
      *type = strType;
      sprintf(result,"%d",event->n);
      return((void *) result);
    }


    /* x,y,w and h variables (mouse local coordinates) */
    if (!strcmp("x",name)){
      *type = strType;
      sprintf(result,"%g",event->x);
      return((void *) result);
    }
    if (!strcmp("y",name)){
      *type = strType;
      sprintf(result,"%g",event->y);
      return((void *) result);
    }
    if (!strcmp("w",name)){
      *type = strType;
      sprintf(result,"%g",event->w);
      return((void *) result);
    }
    if (!strcmp("h",name)){
      *type = strType;
      sprintf(result,"%g",event->h);
      return((void *) result);
    }

    
  }  
   
  return(NULL);   
} 


char GetEventVariable2(char *name, char **str, float *f) 
{
  EVENT event;
  
  *str = NULL;
  
  /* Get event */
  event = toplevelCur->lastEvent;
   
  /* If no event was stored --> NO */
  if (event == NULL) return(NO);
  if (event->type == NoEvent) return(NO);
  
  /*
   * We just test all the  @variables 
   */
      
  /* Key variable */  
  if (!strcmp("key",name)) {
    switch(event->type) {
      case KeyDown : case KeyUp :
        *str = KeyCode2Str(event->key,NO);
        break;
      default : Errorf("GetEventVariable2() : Weird");
    }
    return(YES);
  }

  /* Window variable */  
  if (!strcmp("window",name)) {
    if (event->object != NULL) *str = GetWin(event->object)->name;
    else *str = "";
    return(YES);
  }

  /* Objname variable */  
  if (!strcmp("objname",name)) {
    if (event->object != NULL) *str = GetNameGObject(event->object);
    else *str = "";
    return(YES);
  }     
      
  /* Type variable */
  if (!strcmp("type",name)) {
    switch(event->type) {
      case KeyDown: *str = "keyDown"; break;
      case KeyUp: *str = "keyUp"; break;
      case ButtonUp: *str = "buttonUp"; break;
      case ButtonDown: *str = "buttonDown"; break;
      case Enter: *str = "enter"; break;
      case Leave: *str = "leave"; break;
      case MouseMotion: *str = "mouseMotion"; break;
      case Draw: *str = "draw"; break;
      case Del: *str = "delete"; break;
      case ErrorEvent: *str = "errorEvent"; break;
      default :  Errorf("GetEventVariable2() : event type %d unknown",event->type);
    }
    return(YES);
  }      

  /* Button variable */
  if (!strcmp("button",name)) {
    switch(event->type) {
      case ButtonDown : case ButtonUp: case MouseMotion :
        *str = ButtonCode2Str(event->button); break;
      default :  Errorf("GetEventVariable2() : bad button");
    }
    return(YES);
  }

  /* i, j, m and n variables (mouse global coordinates) */
  if (!strncmp("i",name,1)) {*f = event->i; return(YES);}
  if (!strncmp("j",name,1)) {*f = event->j; return(YES);}
  if (!strncmp("m",name,1)) {*f = event->m; return(YES);}
  if (!strncmp("n",name,1)) {*f = event->n; return(YES);}
  
  /* x,y,w and h variables (mouse local coordinates) */
  if (!strncmp("x",name,1)) {*f = event->x; return(YES);}
  if (!strncmp("y",name,1)) {*f = event->y; return(YES);}
  if (!strncmp("w",name,1)) {*f = event->w; return(YES);}
  if (!strncmp("h",name,1)) {*f = event->h; return(YES);}

  return(NO);
}  
    
    
    
/************************************************************************
 *
 *  Main Function that process bindings
 *
 *    - It process any binding script attached to 'event'
 *    - It returns either NULL or sequence of key code that must be printed on the terminal 
 *    - If it is waiting for a key sequence to be completed it changes
 *      'event' so that its type corresponds to NoEvent
 *
 ************************************************************************/
 
static void Add1Event(void)
{
  /* Here we just count the number of received events */
  if (toplevelCur->nEvents < ULONG_MAX) toplevelCur->nEvents++;
  else toplevelCur->nEvents = 1;
}

static float EvalDelayBinding(void)
{
  BINDING b;
  float delayMax;
  float time,delay;
  
  b = theTerminalBindings[OtherEventCategory];
  
  delayMax = -1; 
  time = MyTime();
  for (;b!=NULL;b= b->next) {
    if (b->group != NULL && b->group->flagActive == NO) continue;
    if (b->eventType == TimeEvent || b->eventType == DelayEvent) {
      if (b->eventType == TimeEvent) delay = b->time-time;
      else delay = b->time+b->delay-time;
      if (delay < 0) {
        delayMax = 0;
        break;
      }
      if (delayMax == -1) delayMax = delay;
      else delayMax = MIN(delayMax,delay);
      continue;
    }
  }
  
  return(delayMax);
}
  
static unsigned long *DoBinding(EVENT event) 
{
  EVENT event1;
  BINDING b,b1;
  BINDING *theBindings;
  int c,i,i1,j;
  char flagFirst;
  GCLASS classCur;
  
  /* The following variables are used for buffering key events for binded KeyDown sequences */
  static unsigned long bKeys[MaxLengthKeySeq];
  static unsigned long termKeys[MaxLengthKeySeq];
  unsigned long *result = NULL;
  static GOBJECT object = NULL;
  static int nKeys = 0;
  unsigned long *r = NULL;
  static int nKeysToPull = 0;
  static char flagKeyIsOk;
    
  /* No Event */  
  if (event->type == NoEvent) {
    if (EvalDelayBinding()==-1) return(NULL);
    event->type = DelayEvent+TimeEvent;
    event->object = NULL;
  }
   
  /*  
   * We should we empty the buffer if an event happened in another object or an event different from a key event 
   * or a mouse motion event happened in the object
   */
  if (nKeys != 0 && (event->object != object || !(event->type & (KeyUp | KeyDown | MouseMotion)))) {  
    if (object == NULL) { /* Case of the terminal */
      memcpy(termKeys,bKeys,sizeof(unsigned long)*nKeys);
      termKeys[nKeys] = 0;
      result = termKeys;
    }
    nKeys = 0;
    nKeysToPull = 0;
  } 
   
  /* If the buffer is empty then we should not pull any keys out of it */
  if (nKeys == 0) nKeysToPull = 0;
  if (event->object != NULL) classCur = event->object->classCur;
    
  /* 
   * If we are dealing with GObject bindings then we need to put the event->key in the buffer if
   * keyDown event moreover we have to set te flag that will tell us if we have a chance to
   * match a binding
   */
   if (event->type == KeyDown && (event->object == NULL || classCur == theGObjectClass)) {
     bKeys[nKeys] = event->key;
     nKeys++;
     flagKeyIsOk = NO;
     object = event->object;
   }
     
  /* We get the corresponding bindings (and set back the current class of the object to its original class) */
  if (event->object == NULL) theBindings = theTerminalBindings;
  else {
    theBindings = event->object->classCur->theBindings;
    event->object->classCur = event->object->gclass;
  }
  
  /* The index in the "theBindings" list */
  c = GetBindingCategory(event->type);
        
  /* We store the event */
  event1 = toplevelCur->lastEvent;
  toplevelCur->lastEvent = event;
            
  /* Loop on the bindings */
  b = theBindings[c];
  if (b != NULL) b->state = BindingOff;
  flagFirst = YES;
  while(1) {
  
    /* Case the first binding is NULL */
    if (b == NULL) break;
  
    /* Should we delete the binding whose script was just executed ? */
    if (b->state == Binding2BeDeleted) {
      b1 = b->next;
      DeleteBinding(b);
      b = b1;
    }
    
    /* If it is not the first time the loop is being executed, we must get the next binding */
    else if (!flagFirst) {
      b->state = BindingOff;
      b = b->next;
    }

    /* If no binding then leave the loop */
    if (b == NULL) break;
    
    /* Next time in the loop will not be the first time */
    flagFirst = NO;
    
    /* Init the state */
    b->state = BindingOff;
    
    /* Go to next binding if binding is not active */
    if (b->group != NULL && b->group->flagActive == NO) continue;

    /* Go to next binding if binding does not match the eventtype */
    if ((b->eventType & event->type) == 0) continue;
    
    /*
     * Buttons 
     */
    if (event->type & (ButtonUp + ButtonDown)) {
    
      /* Go to next binding if binding does not match the right button */ 
      if ((b->button & ModMask) != ModAny && (b->button & ModMask) != (event->button & ModMask)) continue;
      if (!(b->button & ButtonMask & event->button)) continue;
      
      b->state = BindingOn;
      EvalScript(b->script,NO);
      Add1Event();
      InitResult();        
    }
    
    /*
     * Motion
     */
    else if (event->type & MouseMotion) {
    
      /* Go to next binding if binding does not match the right button */      
      if ((b->button & ModMask) != ModAny && (b->button & ModMask) != (event->button & ModMask)) continue;
      if (!(b->button & ButtonMask & event->button)) continue;

      b->state = BindingOn;
      EvalScript(b->script,NO);      
      InitResult("");
      Add1Event();
    }

    /*
     * Enter/Leave
     */
    else if (event->type & (Enter + Leave)) {
          
      b->state = BindingOn;
      EvalScript(b->script,NO);
      Add1Event();
      InitResult();
    }

    /*
     * KeyUp
     */
    else if (event->type & KeyUp) {
    
      /* Go to next binding if binding does not match the right key */
      if (event->key != b->keys[0] && (b->keys[0] != AnyKC || event->key == EscapeKC) &&
          ((event->key & ModMask) != ModAny || (event->key & KeyMask) != b->keys[0])) continue;

      b->state = BindingOn;
      EvalScript(b->script,NO);
      Add1Event();
      InitResult();
    }

    /*
     * KeyDown
     */
    else if (event->type & KeyDown) {
 
      i = b->nKeys;
      while (1) {
      
        /* 
         * Look for the next character (reverse order) in the binding sequence that matches the character
         * that was just typed 
         */
        i--;
        for (;i>=0;i--)  if (bKeys[nKeys-1] == b->keys[i] || (b->keys[i] == AnyKC && event->key != EscapeKC)) break;
        
        /* If there is none then it's over */
        if (i == -1) break;
        
        /* Otherwise we try to match the buffered keys */
        for (i1 = i-1,j = nKeys-2;i1>=0 && j >= 0;i1--,j--) 
          if (bKeys[j] != b->keys[i1] && (b->keys[i1] != AnyKC || bKeys[j] == EscapeKC)) break;
          
        /* If we could not match all the first binded keys (up to the ith) then we go on */
        if (i1 != -1) continue;

        /* If we haven't match all the binding sequence then we should wait */
        if (i != b->nKeys-1) {
          flagKeyIsOk = YES;
          continue;
        }

                        
        /*
         * We need to remember how many keys 
         * have to be pulled out from the buffer.
         */
        nKeysToPull = MAX(nKeysToPull,i+1);

        b->state = BindingOn;
        EvalScript(b->script,NO);
        Add1Event();
        InitResult();
        break;
      }
    }

    /*
     * Draw or Delete Graph event
     */
    else if (event->type & (Draw + Del)) {
    
      b->state = BindingOn;
      EvalScript(b->script,NO);
      Add1Event();
      InitResult();
    }
        
    /*
     * Error event
     */
    else if (event->type & ErrorEvent) {
    
      b->state = BindingOn;
      EvalScript(b->script,NO);
      Add1Event();
      InitResult();
    }

    /*
     * Time events
     */
    else if ((event->type & TimeEvent) && b->eventType == TimeEvent) {
      if (MyTime() >= b->time) {
        b->state = BindingOn;
        EvalScript(b->script,NO);
        InitResult();
        b->state = Binding2BeDeleted;
      }
    }

    /*
     * Delay event
     */
    else if ((event->type & DelayEvent) && b->eventType == DelayEvent) {
      if (MyTime() >= b->time+b->delay) {
        b->state = BindingOn;
        EvalScript(b->script,NO);
        InitResult();
        b->time+=b->delay;
      }
    }

  }

 
  /* 
   * We are dealing (in a loop) first at the bindings of the GObjects
   * then the bindings of the class below it and so on
   * down to the class of the object.
   * If this is the case and the event->type is a keyDown, then we need
   * to pull out some keys from the buffer.
   * If there is none we have to check the flagKeyIsOk.
   */
  if (event->type == KeyDown && (event->object == NULL || classCur == event->object->gclass)) {

    if (nKeysToPull == 0) {
      if (!flagKeyIsOk) { /* We empty te buffer */
        if (object == NULL) { /* case of the terminal */
          memcpy(termKeys,bKeys,sizeof(unsigned long)*nKeys);
          termKeys[nKeys] = 0;
          result = termKeys;
        }
        nKeys = 0;
      }
      else event->type = NoEvent;
    }
    else {
      if (nKeysToPull > nKeys) Errorf("DoBindings() : Weird error");
      /* We pull the keys */
      for (i=0,j=nKeysToPull;j<nKeys;i++,j++) bKeys[i] = bKeys[j];
      bKeys[i] = 0;
      nKeys = i;
      nKeysToPull = 0;
      event->type = NoEvent;
    }
    
  }

  /* We restore the event */ 
  toplevelCur->lastEvent = event1;

  return(result);
}



/**************************************************************************
 * 
 *  This function is just for debugging 
 *
 **************************************************************************/

/* #define DEBUGEVENT 1  */
 
static void PrintEvent(EVENT event)
{
  int i = 0;
  
    if (event->object == NULL) return;
    
    switch(event->type) {
 
    case ButtonDown:
       Printf("ButtonDown : GObject %s (%s),  x = %g, y = %g [%d %d], button = %d mod = %d\n",
	      event->object->name,event->object->classCur->name,event->x,event->y,event->i,event->j,event->button & ButtonMask, event->button & ModMask);
       break;
    case ButtonUp:
       Printf("ButtonUp : GObject %s  (%s),  x = %g, y = %g [%d %d], button = %d mod = %d\n",
	      event->object->name,event->object->classCur->name,event->x,event->y,event->i,event->j,event->button & ButtonMask, event->button & ModMask);
       break;
    case Resize:
       Printf("Resize : GObject %s  (%s),  x = %g, y = %g, w = %g, h = %g [%d %d %d %d]\n",
	      event->object->name,event->object->classCur->name,event->x,event->y,event->w,event->h,event->i,event->j,event->m,event->n);
       break;
    case Enter:
       Printf("Enter : GObject %s (%s) x= %g  y = %g\n", event->object->name,event->object->classCur->name,event->x,event->y);
       break;
    case Leave:
       Printf("Leave : GObject %s (%s)\n", event->object->name,event->object->classCur->name);
       break;
    case KeyDown:
       Printf("KeyDown : GObject %s  (%s), x = %g, y = %g [%d %d] key = %d, (%c)\n",
	      event->object->name,event->object->classCur->name,event->x,event->y,event->i,event->j,event->key,event->key);
       break;
    case KeyUp:
       Printf("KeyUp : GObject %s  (%s), x = %g, y = %g [%d %d] key = %d, (%c)\n",
	      event->object->name,event->object->classCur->name,event->x,event->y,event->i,event->j,event->key,event->key);
       break;
    case MouseMotion:
       Printf("MouseMotion : GObject %s  (%s),  button = %d, x = %g, y = %g [%d %d] mod = %d\n",
	      event->object->name,event->object->classCur->name,event->button & ButtonMask,event->x,event->y,event->i,event->j,event->button & ModMask);
       break; 
    case Draw:
       Printf("Draw : GObject %s  (%s) x = %g, y = %g, w = %g, h = %g [%d %d %d %d]\n",
	      event->object->name,event->object->classCur->name,event->x,event->y,event->w,event->h,event->i,event->j,event->m,event->n);
       break;
    case Del:
       Printf("Delete : GObject %s (%s)\n", event->object->name,event->object->classCur->name);
       break;
  }
}

    
/************************************************************************
 *
 *  Main Function for processing the next event
 *
 ************************************************************************/

/*
 * Static variables for remembering the last object that was visited by the mouse
 * and the object the user has pushed the mouse button in (if any)
 */
static GOBJECT objButton = NULL;
static GOBJECT objLast = NULL;
static int depthLast = 0;
static float xLast,yLast;
static int iLast, jLast;


/*
 * Subroutine called by 'Process1Event_' and 'SendEvent' only that processes the event 'event'
 */
static int Process1Event__(EVENT event)
{
  unsigned long *termKeys,*k;
  GCLASS c[100],class;
  int n;
  WINDOW w;
  
  
#ifdef DEBUGEVENT
 PrintEvent(event); 
#endif

  /* First we get the object class hierarchy in the array 'c' */
  c[0] = NULL;
  n = 0;
  if (event->type != NoEvent && event->object != NULL) {
    class = event->object->classCur;
    /* Get the class hierarchy in the c array */
    if (event->type == Del) c[n] = class;
    else {
      c[n] = class;
      while (c[n]->fatherClass != NULL) {
        c[n+1] = c[n]->fatherClass;
        n++;
      }
    }
  }

  /* Set the cliprect */
  if (event->type != NoEvent && event->object != NULL && event->type != Draw && event->type != Del && event->type != Resize) {
    w = GetWin(event->object); 
    if (((GOBJECT) w) != event->object) WSetClipRect(w,event->object->x,event->object->y,event->object->w,event->object->h);
    else WSetClipRect(w,0,0,event->object->w,event->object->h);
  }

  /* Process any binding associated to this event (loop on the father classes) */    
  while (1) {
    if (event->type != NoEvent && event->object != NULL) {
      event->object->classCur = c[n]; 
    }
    termKeys = DoBinding(event);
    n--;
    
    /* Should we print some keys on the terminal ? */
    if (termKeys != NULL) 
      for (k=termKeys;*k != 0;k++) TermBufferPushKey(*k);
    
    if (n < 0) break;
  }

  /* If No event then return */
  if (event->type == NoEvent) return(NoEvent);  

  /* Process KeyDown event in the terminal window */
  if (event->type == KeyDown && event->object == NULL) {
      Add1Event();
     return(KeyDown);
      TermBufferPushKey(event->key);
      Add1Event();
      return(event->type);
  }

  /* If a delete event we must check whether the variables "objButton", "objLast" are pointing to it */
  if (event->type == Del) {
    if (objLast == event->object) objLast = NULL;
    if (objButton == event->object) objButton = NULL;
    /* ?? The following line should be improved there is a big danger !! if the object deleted
       is in the past events of toplevelCur->lastEvent */
    if (toplevelCur->lastEvent && toplevelCur->lastEvent->object == event->object) toplevelCur->lastEvent = NULL;
  }
  
  return(event->type);
}



/*
 * Extern function for sending a draw/delete event directly from a C function
 */
void SendEvent(EVENT event) 
{
  Process1Event__(event);
}


/*
 * Subroutine called by 'Process1Event' only that processes the event 'event'
 * that was received from the main event queue
 * It processes all events except refresh/delete and resize of windows. These events 
 * were prcoessed by the 'Process1Event_' function
 * Moreover it takes care of the 'Enter/Leave' events and the fact that when a button
 * is pushed in a gobject it is grabbed by this gobject until it is realeased.
 */

static int Process1Event_(EVENT event)
{
  
  GOBJECT o,*obj1,objArray[100];
  int d,depthCur;
  char flag1,flag2,flag3,flag4;
  float x,y;
  struct event event1;
  
  /* if a button up is detected and no button down then just ignore it */
  if (event->type == ButtonUp && objButton == NULL) event->type = NoEvent;

  /*
   * Case a button is being grabbed by a gobject
   * All the coordinates must be computed acording to that gobject and no enter/leave events are sent
   */   
  if (event->type & (ButtonUp+MouseMotion) && objButton != NULL) {
    Global2Local(objButton,event->i,event->j,&x,&y);    
    event->x = x;
    event->y = y;
    event->object = objButton;
    objButton->classCur = objButton->gclass;
    if (objLast != objButton) {
      objLast = objButton;
      depthLast = 0;
      for(o = objLast;o != NULL;o=(GOBJECT) (o->father),depthLast++);
    }
    xLast = event->x;
    yLast = event->y;
    iLast = event->i;
    jLast = event->j;
    if (event->type == ButtonUp) objButton = NULL;
    return(Process1Event__(event));
  }


  /*
   * Case no button is being grabbed by a gobject and an event (!= NoEvent) was received
   */     
  if (event->type != NoEvent) {
    
    /* Finds which object the mouse is in and where ? */
    if (event->object != NULL) {

      d = IsInGObject(event->object,&o,event->i,event->j);

      if (d < 0 || o == NULL) o = event->object;

      Global2Local(o,event->i,event->j,&x,&y);    

      event->object = o;
      o->classCur = o->gclass;
      event->x = x;
      event->y = y;

      /* Compute the depth of the current object */
      if (event->object == objLast) depthCur = depthLast;
      else {
        depthCur = 0;
        for(o = event->object;o != NULL;o=(GOBJECT) (o->father),depthCur++);
      }
    }

    /* 
     * There are four cases between objLast and event->object for managing enter/leave events
     *
     *  1- (flag1 == YES) They are the same (no event to be sent) 
     *  2- (flag2 == YES) objLast is an ancestor of event->object (just enter events)
     *  3- (flag3 == YES) event->object is an ancestor of objLast (just leave events)
     *  4- (flag4 == YES) all the other cases (whole bunch of enter and leave events)
     *
     */ 
    
    flag1 = flag2 = flag3 = flag4 = NO;

    /* Case 1 ? */
    if (objLast == event->object) flag1 = YES;
    else {

      /* Case 2 ?  */
      for (o = event->object, d = depthCur;o != NULL && o != objLast && d >= depthLast;o = (GOBJECT) (o->father),d--);
      if (o == objLast && o != NULL) flag2 = YES;
 
       /* Case 3 ?  */
      for (o = objLast, d = depthLast;o != NULL && o != event->object && d >= depthCur;o = (GOBJECT) (o->father),d--);
      if (o == event->object && o != NULL) flag3 = YES;

      /* Case 4 ? */
      if (!flag2 && !flag3) flag4 = YES;
    }
       
    /* Send some leave events ? */
    if ((flag3 || flag4) && objLast != NULL)  {
      for (o = objLast;o != NULL && o != event->object; o = (GOBJECT) (o->father)) {
        event1.type = Leave;
        event1.object = o;
        event1.object->classCur = event1.object->gclass;
        Process1Event__(&event1);
      }
    }
      
    /* Send an enter event */      
    if ((flag2 || flag4) && event->object != NULL) {
      obj1 = objArray;
      for (o=event->object; o != NULL && o != objLast; o = (GOBJECT) (o->father)) {
        *obj1 = o;
        obj1++;
      }
      obj1--;
      event1.m = 1;
      for (;obj1 >= objArray;obj1--) {
        event1.type = Enter;
        event1.object = *obj1;
        event1.object->classCur = event1.object->gclass;
        event1.i = event->i;
        event1.j = event->j;
        Global2Local(*obj1,event->i,event->j,&(event1.x),&(event1.y));
        Process1Event__(&event1);
        event1.m = 0;
      }
    }
      
    objLast = event->object;
    depthLast = depthCur;
    xLast = event->x;
    yLast = event->y;
    iLast = event->i;
    jLast = event->j;

    if (event->type == Enter || event->type == Leave) event->type = NoEvent;
  }
    
  if (event->type == ButtonDown) {
  objButton = event->object;
  }

  return(Process1Event__(event));
}
    
/*
 * Subroutine called by 'ProcessNextEvent' only that processes the event 'event'
 * that was received from the main event queue
 * It processes refresh/delete and resize of windows. All the other events 
 * are sent to 'Process1Event_'
 */
static int Process1Event(EVENT event)
{    
  switch(event->type) {
  
    case Draw :  /* Process refresh event if asked */
      if (((WINDOW) (event->object))->flag == WindowFlagNoUpdate) 
        ((WINDOW) (event->object))->flag = WindowNoFlag;
      else DrawGObject(event->object,event->i,event->j,event->m,event->n,NO);
      break;
  
    case Del :  /* Process delete window if asked */
      DeleteGObject((GOBJECT) event->object);
      break;
      
    case Resize :  /* Process resize window if asked */
      ((WINDOW) (event->object))->flag = WindowFlagNoChangeFrame;
      MoveResizeDrawGObject((GOBJECT) event->object,event->i,event->j,event->m,event->n);
      ((WINDOW) (event->object))->flag = WindowNoFlag;
      break;
    
    case Leave :
      event->object = NULL;
        
    default : return(Process1Event_(event));
    
  }

  return(event->type);
}



/*
 * Extern function for processing the next 'event' on the event queue
 */ 
int ProcessNextEvent(int flagWait)
{  
  static int i = -10;
  static int j = -10;
  struct event event;
  
  /* We get the next event */
  WGetNextEvent(&event,flagWait);

  /* Correction of mouse moving event */
  if (event.type == MouseMotion) {
    if (i == event.i+event.object->x && j == event.j+event.object->y) {
      event.type = NoEvent;
    }
    else {
      i = event.i+event.object->x;
      j = event.j+event.object->y;
    }
  }
 
  if (event.type != NoEvent && event.object != NULL) event.object->classCur = event.object->gclass;
  
  return(Process1Event(&event));
}

void C_Event(char **argv)
{
  char *action;
  int f;
  float t;
  
  argv = ParseArgv(argv,tWORD,&action,-1);
  
  if (!strcmp(action,"process")) {
    argv = ParseArgv(argv,tFLOAT_,0.0,&f,0);
    if (f<0) ErrorUsage();
    if (f == 0) {
      while (ProcessNextEvent(NO) != NoEvent);
    }
    else {
     t= MyTime();
     while (MyTime()-t > f) ProcessNextEvent(NO);
    }
  }
  
  else Errorf("Bad Action '%s'\n",action); 
}


    
    
    
    
    
    
    



 


