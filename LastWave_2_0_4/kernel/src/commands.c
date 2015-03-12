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



/****************************************************************************/
/*                                                                          */
/*  commands.c  : This file defines all the basic commands                  */
/*                   The commands are grouped in different 'tables'         */
/*                   Each command is described by 3 fields :                */
/*                           name of the command                            */
/*                           name of the C-function it corresponds to       */
/*                           a string describing the list of arguments      */
/*                                 required and a one-line help             */
/*                                                                          */
/****************************************************************************/



#include "lastwave.h"




/************************************************************
 *          CProcs related to the command language
 ***********************************************************/

/* In int_history.c */
extern void C_History();

/* In int_variables.c */
extern void C_Set(),C_Var(),C_Import(),C_Global(),C_SetVar(),C_Type(),C_New(),C_Delete(),C_Copy(),C_Clear();

/* In int_hash.c */
extern void C_Array();

/* In int_misc.c */
extern void C_SystemUnix(),C_Echo(),C_Time(),C_RandInit();

/* In int_expr.c */
extern void C_Setv(),C_Info(),C_Print(),C_Val();

/* In int_controls.c */
extern void C_For(),C_Foreach(),C_Break(),C_Continue(),C_Return(),C_Returnv(),C_If();
extern void C_While(),C_Do();

/* In int_streams.c */
extern void C_GetLine(),C_GetChar(),C_Errorf(),C_Printf(),C_SPrintf(),C_SScanf(),C_Scanf();
extern void C_Terminal(),C_File();

/* In int_eval.c */
extern void C_Eval();

/* In int_alloc.c */
extern void C_List(),C_Listv(),C_Str(),C_DebugAlloc();

/* In int_commands.c */
extern void C_SetSourceDirs(),C_Source(),C_SetProc(),C_Proc(),C_Apply();

/* In int_package.c */
extern void C_Package();

CProc interpreterTable[] = {

  /*
   * CProcs in int_history.c 
   */
  "history", C_History,"{{{} {Displays the history of the commands (i.e., the \
last commands that were typed in the terminal along with their index number).}} \
{{index [<index>]} {If <index> is specified, it returns the history command \
associated to the <index> number in the history. If not, it returns the current index number \
(of the next entry in the history).}} \
{{match [<regexp>='*']} {Gets all the history lines that matches <regexp>. The result is given as a listv which \
goes from the most recent command to the eldest one.}} \
{{size [<size>]} {Gets/Sets the size of the history.}}}",
  "h", C_History,"{{{} {Short name for the command 'history'.}}}",
  
  /*
   * CProcs in int_variables.c 
   */
  "set",C_Set,"{{{Old Lastwave 1.7 command} {Not to be used.}}}",
  "setvar",C_SetVar,"{{{Old Lastwave 1.7 command} {Not to be used.}}}",

  "new",C_New,"{{{*&type*} {Returns a new variable of type <type>.}}}",
  "delete",C_Delete,"{{{[<level>=0] *varName*} {Deletes a variable.}}}",

  "copy",C_Copy,"{{{<val> [<val1>]} {Copy of the value <val> in <val1> if specified otherwise returns a copy.}}}",
  "clear",C_Clear,"{{{<val>} {Clears the content of a value.}}}",

  "type",C_Type,"{{{exist <&type> [-s]} {Returns 0 if type <&type> does not exist and 1 otherwise. If '-s' is on, it tests wether this type \
is associated to a type structure (which is not the case, for instance, of the type '&val').}} \
{{list [<packageName>]} {Returns the listv of all the names of the available types. If <packageName> is specified, only types of the package are returned.}} \
{{field <&type> [<regexp>='*'] [-sge]} {Returns the listv of the field names associated to the type <&type>. The names must match <regexp>. \
If '-s' is on, only write-enabled fields are returned. If '-g' is on, only read-enabled fields are returned. \
If '-e' is on, only extract-enabled fields are returned.}} \
{{help <&type> [<field> | -n]} {If no argument, it returns the documentation about the type <&type>. If argument <field>, it returns the listv of documentation on the field <field> of the type <&type>. The first element is the Get documentation, \
the second is the Set documentation, and the third one is the extract option documentation. If the corresponding method \
does not exist 'null' is put in the listv. In the case <field> is replaced by '-n', this command returns the documentation concerning \
number extraction (e.g., 10a) if number extraction is permitted or null if not.}}}",

  "array",C_Array,"{{{new [<hashSize>=8]} {Creates a new &array variable using a hash table of size <hashSize>.}} \
{{list <array> [<regexp>='*']} {Gets the listv of indexes of <array> that matches <regexp>.}}}",

  "var",C_Var,"{{{unix <unixVariable>} {Gets the string value of a unix shell variable (on unix computers only).}} \
{{exist [<level>=0] *[@]varName* [<&type>]} {Tests whether a variable (it can ONLY be a simple variable like 'me' or a variable \
stored in an array like 'me.house.kitchen') of type <&type> exists in level <level>. \
It returns 1 if it does not exist and 0 otherwise. \
It works with @variables too (in that case <level> is not used).}} \
{{list [<level>=0] [<regexp>=[^_]*] [<&type>]} {Gets the list (not a listv -> this will be changed soon) of variable names of level <level> and type <&type> whose name matches <regexp>.}} \
{{env [<level>=0] [<regexp>=[^_]*] [<&type>]} {Gets a list made of 3 or 2 element lists corresponding to each variable of level <level> \
and type <&type> whose name matches <regexp>. Each of this list is made of the name of the variable, its type and in the case \
it is a number or a string its value.}} \
{{delete [<level>=0] <var1> [<var2>...<varN>]} {Deletes N variables from level <level>.}} \
{{type [<level>=0] <var> [<&type>]} {Gets the type of a variable from level <level>. It can ONLY be a simple variable like 'me' or a variable \
stored in an array like 'me.house.kitchen'. If it does not exist then it returns the empty string. Otherwise it returns its type.}}}",

  "import",C_Import,"{{{args [<level>=-1] [*&type*] `*varName1* [*newVarName1*]` ... `*varNameN* [*newVarNameN*]`} \
{Imports N variables of type *&type* from level <level> whose names are *varName1*...*varNameN* \
and eventually renames them in the local environement with names \
*newVarName1*...*newVarNameN*. If these new names are not specified then in the case of non array variables, the same names are used. \
When the array syntax is involved to refer to any variable *var1* to *varN* (e.g., 'people.john.adress') and if no new name is \
specified, then the last index \
name is used (in our case the variable will be named 'adress' in the local environment). \
If the type is &var or &array and if any variable does not exist at level <level> it will create it \
in both the level <level> and the local environment. \
If it fails then an error occurs unless [?] is specified right after the <level> in which case it returns 0 instead of 1.}} \
{{list [<level>=-1] {{[*&type1*] *varName1* [<varDef1>]} ... {[*&typeN*] *varNameN* [<varDefN>]}} <list>} {This action does exactly what happens \
when a script procedure is called : the <list> of arguments (evaluated in the calling level) are matched to the variable pattern list.}} \
{{listv {{[*&type1*] *varName1* [<varDef1>]} ... {<*&typeN*> *varNameN* [<varDefN>]}} <listv>} {Same as the 'list' action except that a list of values is \
passed instead of a string list (which is evaluated).}}}",
  "global",C_Global,"{{{[*&type*] `*varName1* [*newVarName1*]` ... `*varNameN* [*newVarNameN*]`} \
{Same as the 'import args' command except that it imports from the global level.}}}",
   
  /*
   * CProcs in int_misc.c 
   */
  "shell",C_SystemUnix,"{{{*unixCommand*} {Executes an unix shell command (for Unix computers only). You can also use the syntax '! *unixCommand*'. Warning : \
Do not use any interactive unix command (such as 'more'), it will core dump lastwave !}}}",
  "echo",C_Echo,"{{{*arg1* ... *argN*} {Just echoes each of its arguments (no evaluation) on standard output.}}}",
  "time",C_Time,"{{{} {Gets the number of seconds since lastwave started.}}}",
  "randinit",C_RandInit,"{{{[<init>]} {Initializes the random generator. If <init> is not specified it initializes it using the computer time. If not, it must be a positive \
integer.}}}",
  
  /*
   * CProcs in int_expr.c 
   */
  "setv",C_Setv,"{{{[*var* | *list_of_var*] [[:*+/-^]=] <value> [-l <level>]} {This is the main evaluator command. The command 'a+=b' is equivalent to \
'setv a += b' and 'a=b' is equivalent to 'setv a = b' or also 'setv a b'. The only advantage of the 'setv' syntax is that it lets you specify \
a <level> in which the evaluation takes place (not the assignement !!).}}}",
  "info",C_Info,"{{{<value>} {Prints info on a value.}}}",
  "print",C_Print,"{{{<value1> [-s | <value2> ... <valueN>]} {Prints one or seveal values. Option '-s', prints it the 'short' way.}}}",
  "val",C_Val,"{{{test *exprString* [<&type>] [-l <level>] [-E]} {Returns the listv '{<type> <result>}' if the *exprString* evaluates successfully to <result> (which must \
be of type <&type> if argument <&type> is specified). If evaluation is not successfull it returns 'null' unless '-E' is set in which case it generates an error. \
Option '-l' lets you specify a level in which evaluation takes place.}} \
{{eval <val> [-l <level>]} {Evals the value <val> and returns it.}} \
{{type <val> [-b] [-l <level>]} {Returns the type of <val>. If '-b' then the 'basic' type is returned, e.g., &signal is returned instead of &signali. Option '-l' lets you specify a level in which evaluation takes place.}}}",

  /*
   * CProcs in int_controls.c
   */
  "if", C_If, "{{{<test> <thenScript> [elseif <test1> <thenScript1> ..... elseif <testN> <thenScriptN> else <elseScript>]} \
{The standard 'if then else' control. WARNING: each memeber of the <test> is evaluated (even if || or && clause).}}}",
  "for", C_For,"{{{<startScript> <continueValue> <nextScript> <bodyScript>} {The 'for' control loop. \
The <startScript> is executed just once at the beginning of the loop. The loop is continued while <continueValue> is evaluated \
to a number different from 0. The <nextScript> is executed at the end of each loop (just before <continueTest> is evaluated) \
and <bodyScript> is the core of the loop.}}}",
  "foreach", C_Foreach,"{{{<var> (<list> | <listv> | <signal> | <range>) <bodyScript>} {The variable <var> loops successively  on each element of the <list>, <listv> \
<signal> or <range> till \
its end is reached. After each assignation <bodyScript> is executed.}}}",
  "while",C_While,"{{{<continueTest> <bodyScript>} {The standard 'while' control loop. At the begining of each loop, \
<continueTest> is evaluated using '=', if it returns 0 then the loop stops otherwise <bodyscript> is executed and the loop goes on.}}}",
  "do",C_Do,"{{{<bodyScript> <continueTest>} {The standard 'do' control loop. At the end of each loop, \
<continueTest> is evaluated using '=', if it returns 0 then the loop stops otherwise <bodyscript> is executed and the loop goes on \
(thus <bodysScript> is always executed at least once).}}}",
  "break", C_Break,"{{{} {Gets immediately out of any 'do', 'while', 'for' or 'foreach' loop.}}}",
/*  "return", C_Return,"{{{[*value*]} {Exits right away from a script procedure and returns *value* (not evaluated) if specified otherwise it returns the null pointer.}}}", */
  "return", C_Returnv,"{{{[<value>] [-e]} {Exits right away from a script procedure and returns <value> if specified otherwise it returns the null pointer. If option '-e' is on, <value> is not evaluated.}}}",
  "continue", C_Continue,"{{{} {Skips directly to the next loop of a 'for', 'foreach', 'while' or 'do' loop.}}}",

  /*
   * CProcs in int_streams.c
   */  
  "getchar",C_GetChar,"{{{[*var*]} {Reads from stdin a character and returns it or sets it in variable *var* if specified.}}}",
  "getline",C_GetLine,"{{{[*var*]} {Reads from stdin a line (ended by a newline or an eof) and returns it or sets it in variable *var* if specified.}}}",
  "scanf",C_Scanf,"{{{<format> [*var1* ... *varN*]} {Same syntax as 'scanf' in C-Language : reads arguments from stdin according to <format> and puts them in the variables *var1*...*varN*. \
Warning : Most of the '%' C-format can be used (but not too complex ones !).}}}",
  "sscanf",C_SScanf,"{{{<string> <format> [*var1* ... *varN*]} {Same syntax as 'sscanf' in C-Language : reads arguments from <string> according to <format> and puts them in the variables *var1*...*varN*. \
Warning : Most of the '%' C-format can be used (but not to complex ones !).}}}",
  "terminal",C_Terminal,"{{{movewindow <x> <y>} {Moves terminal window to position <x> <y> (Warning : only on Macintosh computers).}} \
{{resizewindow <w> <h>} {Resizes terminal window using <w> <h> (Warning : only on Macintosh computers).}} \
{{eraseline} {Erases current terminal line.}} \
{{erasechars [<nChars>=1]} {Erases <nChars> characters from cursor position on current terminal line.}} \
{{movecursor <nSpaces>} {Moves cursor <nSpaces> forward (>0) or backward (<0) from cursor position.}} \
{{prompt [<promptProc>]} {If no argument it gets the current prompt otherwise it sets the procedure that is supposed to return the prompt string.}} \
{{cursor <position>} {Sets the cursor to an absolute position on the current terminal line.}} \
{{insert <string>} {Inserts a string on current terminal line from cursor position.}} \
{{beep} {Just makes the terminal beep.}} \
{{line [<str>]} {Gets/Sets the current terminal line.}} \
{{mode} {Gets the current mode of the terminal. It is either : 'getchar' (while the terminal waits for a single character), 'scanline' \
(while the terminal waits for a whole line to be typed in but not for a command line), 'command' (when the terminal waits for a \
command line to be typed in, in front of the prompt).}} \
{{eof} {Sends an 'eof' character to the terminal.}}}",
  "printf",C_Printf,"{{{<format> [<val1> ... <valN>]} {Same syntax as 'printf' in C-Language : prints on stdout the values <val1>...<valN> according to <format>. \
Warning : Most of the '%' C-format can be used (but not to complex ones !). Moreover two other formats '%V' (resp. '%v') can be used to print the long (resp. short) string representation \
of a value.}}}",
  "sprintf",C_SPrintf,"{{{*var* <format> [<val1> ... <valN>]} {Same syntax as 'sprintf' in C-Language : sets variable *var* according to <format> with string variables <val1>...<valN>. \
Warning : Most of the '%' C-format can be used (but not to complex ones !). Moreover two other formats '%V' (resp. '%v') can be used to print the long (resp. short) string representation \
of a value. It returns the formatted string.}}}",
  "errorf",C_Errorf,"{{{<format> [<val1> ... <valN>]} {Same as 'printf' but prints on stderr instead of stdout and generates an error right after printing.}}}",
  "file",C_File,"{{{cd <directory>} {Changes the working directory (Warning : only on Unix computers).}} \
{{info <file>} {Returns a 2 element listv. The first element is the type of the file (either 'directory', 'file' or 'unknown') and the second is its size. If the file does not exist it returns null. (WARNING : the size is wrong on Macintoshes.)}} \
{{createdir <path> <dirName>} {Creates a directory named <dirName> in directory <path>.}} \
{{list <regexp>} {Returns the listv of filenames whose filename matches <regexp>.}} \
{{listp <regexp>} {Same as 'list' but returns complete paths.}} \
{{tmp} {Gets a filename that does not exist and that can be used for a temporary file.}} \
{{remove <filename>} {Removes a file.}} \
{{move <filenameSrc> <filenameTarget>} {Changes the name of a file.}} \
{{open <filename> ('r' | 'w' | 'a')} {Opens a file in a 'r'ead, 'w'rite or 'a'ppend mode and returns a stream number associated to that file.}} \
{{exist <filename> [<mode>='r']} {Tests whether a file named <filename> could be opened using mode <mode>.}} \
{{openstr <str>} {Opens a stream associated to a string. It returns the stream number.}} \
{{close <stream>} {Closes a stream.}} \
{{eof [<stream>=stdin]} {Returns 1 if <stream> has reached an end of file.}} \
{{set (stdin | stdout | stderr) [<stream>]} {Gets or Redirects 'stdin', 'stdout' or 'stderr' streams.}}}",

  /*
   * CProcs in int_eval.c
   */    
  "apply",C_Apply,"{{{listv [<level>=0] <proc> <listv>} {Apply the procedure <proc> with the argument list being the successive values of the <listv> . \
If <level> is specified (and not 0) then the procedure is executed in a different level (not in the current level.}} \
{{args [<level>=0] <proc> <arg1> ... <argN>} {Apply the procedure <proc> with the specified arguments. \
If <level> is specified (and not 0) then the commands is executed in a different level (not in the current level.}}}",  
  "eval",C_Eval,"{{{[<level>=0] <script>} {It evals the script in the current level. \
If <level> is specified (and not 0) then the script is evaluated in a different level (not in the current level).}}}",

  /*
   * CProcs in int_alloc.c
   */    
  "listv",C_Listv,"{{{map <listv> <proc>} {Maps the procedure <proc> on each element of <listv> and returns \
the listv of the results. The procedure <proc> should take one argument and return a value that will be inserted \
in the result listv. If it does not return anything, nothing is insterted in the listv}} \
{{niceprint <listv> [<colSize>]} {Prints each element of the listv using tabulated columns.}} \
{{sort <listv> [<proc>] [-i]} {Returns the sorted listv. the flag '-i' just reverse the resulting listv. \
If no <proc> are given, the <listv> must be composed either only of numbers (the sorting is then performed in the increasing order) \
or only by strings (the sorting is alphabetical). An error occurs in any other case. If <proc> is specified, it is supposed to be \
the comparaison procedure. It must take 2 arguments arg1 and arg2 and must return an integer (negative if arg1<arg2, 0 if arg1==arg2 \
and positive otherwise}}}",
  "list",C_List,"{{{Old LastWave 1.7 command} {Not to be used.}}}",
  "str",C_Str,"{{{inter <str> <str1>} {Get the common substring which starts both strings}} \
{{match <str> <regexp> [<nOcc>=-1]} {Tests whether some substrings of <str> match the <regexp>. It returns a listv of ranges which correspond to \
non overlapping maximal substrings which match the <regexp>. <nOcc> is the maximum number of occurence it should return (if negative then all are returned). \
The wild cards characters for regexp are \n\
^: beginning of string \n\
$: end of string \n\
?: a single character \n\
+: one character or more \n\
*: zero character or more \n\
[+...]: one character (or more) that follows \n\
[+^...]: one character(or more) that is different from the characters that follow \n\
[#...]: zero or one character that follows \n\
[#^...]: zero or one that is different from the characters that follow \n\
[*...]: zero character (or more) that follows \n\
[*^...]: zero character(or more) that is different from the characters that follow \n\
|...|: range delimiters \n\
!...!: escaped sequence}} \
{{substr <str> <substr> [<nOcc>=-1]} {Same as action 'match' but optimized in the case the <regexp> is a simple string with no wild card.}} \
{{2ascii <str>} {Gets a listv made of the ascii codes corresponding to the characters of the string <str>.}} \
{{ascii2 <listv_of_codes>} {Gets the string associated to a listv of ascii codes (this action is the reverse action of the 'ascii' action).}}}",

  /*  
   * CProcs in int_procs.c 
   */    
  "setsourcedirs",C_SetSourceDirs,"{{{[<listv of directories>]} {Sets/Gets the list of directories the 'source' command looks a file in.}}}",
  "source",C_Source,"{{{*filename1*...*filenameN*} {Source each file *filename1*...*filenameN*.}}}",    
  "setproc",C_SetProc,"{{{(*name* | -) {{[*&varType1*] *varName1* [<varDef1>]} ... {[*&varTypeN*] *varNameN* [<varDefN>]}} \
[\"{{{*usageAction1*} {*helpAction1*}} ... {{*usageActionN*} {*helpActionN*}}}\"] <bodyScript>} \
{Defines a new script procedure whose name is *name* (a non evaluated string) and returns it. \
If the name is '-' then the created procedure is anonymous (you can create anonymous procedure using the syntax '%{a b}`script`' too). \
The variables of the procedure are defined as a list where the type of each variable *&varTypeN* can be specified  along with a default value <varDefN> \
if the argument is optional. When no type is specified the '&val' type is used (i.e., evaluation will take place with no apriori on the type of the result). \
The last variable can be of the dotted form (e.g., .l) in which \
case all the remaining arguments will be affected to this argument. There are 2 cases : if the type '&wordlist' is specified for the dotted \
variable, the remaining arguments are not evaluated and the dotted variable \
will be a string list made of the argument names. \
If no type is specified each remaining argument is evaluated and the so-obtained values are stored in the dotted variable as a '&listv'. \
One can specify usage form and a one line help. A simple form is for instance \"{{{<arg1> <arg2>} {The one line help}}}\". \
If the procedure admits several 'actions' (as the 'var' C-procedure for instance), a simple form is \
for instance \"{{{action1<arg1> <arg2>} {The one line help for action1}} \
{{action2 <arg1> <arg2>} {The one line help for action2}}}\".}}}",
  "proc",C_Proc,"{\
{{undef <name1>  [<name2> ... <nameN>]} {Removes N script procedures from the procedure table. If any variable still \
points to the procedure, the procedure becomes 'anonymous', consequently its name name is changed to an anonymous name.}} \
{{Old Lastwave command} {Not to be used.}} \
{{get <proc_name>} {returns the script procedure or the C-procedure whose name is <proc_name>. The type of the result is '&proc'.}} \
{{var <sc_name>} {Gets the variable list of a script command.}} \
{{ctable [<tableNameRegexp>='*'] [<packageNameRegexp>='*']} {Gets the listv of all the C-procedure tables that belongs to a \
package whose name matches <packageNameRegexp>.}} \
{{list [<nameRegexp>='[^_]*'] [<tableNameRegexp>='*'] [<packageNameRegexp>='*'] [-m]} {Gets the listv of all the procedure names which match <nameRegexp> and (in the case of C-commands) which belongs to \
a procedure table whose name matches <tableNameRegexp>. If '-m' is on then it looks for a command named <nameRegexp> (exact string). The listv is organized as 2 sublistv, one for script commands and one for C-commands}} \
{{clist [<nameRegexp>='[^_]*'] [<tableNameRegexp>='*'] [<packageNameRegexp>='*'] [-m]} {Same as 'list' action but looks for C-commands only.}} \
{{slist [<nameRegexp>='[^_]*'] [<tableNameRegexp>='*'] [<packageNameRegexp>='*'] [-m]} {Same as 'list' action but looks for script commands only.}}}",

  /*  
   * CProcs in int_package.c 
   */    
  "package",C_Package,"{{{load [<regexp>='*']} {Load the packages whose name matches <regexp>.}} \
{{name [<regexp>='*']} {Gets the list of all the package whose name matches <regexp>.}} \
{{new <name> <year> <version> <authors> <info>} {Creates a new package named <name> that is copyrighted by <authors> and which was \
written in year <year> and whose version number is <version>. <info> is a text decribing the package.}} \
{{dir [<directory>]} {Sets/Gets the directory where all the package script files are to be searched. Whenever a package named 'name' \
is loaded, the source directory '<directory>/name' will be added \
and the script file <directory>/name.pkg is sourced if it exists.}} \
{{list [<regexp>='*']} {Gets the listv of all the available packages. Each element of the listv is a listv with the name of the package, a flag indicating whether the corresponding \
package is loaded (flag==1) or not (flag==0), the year the package has been written, the version number, a list of authors and a one-line help.}}}",

  
   NULL,NULL,NULL
};


/************************************************************
 *          CProcs related to graphics
 ***********************************************************/

/* In window.c */
extern void C_Window(),C_Msge(),C_Setg(),C_SetgU(),C_GClass(),C_GUpdate();

/* In color.c */
extern void C_Color(), C_SetColor(),C_ColorMap(); 

/* In window_manager.c */
extern void C_Draw(),C_Font();

/* In macwindow.c or X11window.c */
extern void C_System();

/* In event.c */
extern void C_SetBinding(),C_Binding(),C_Event();

/* In postscript.c.c */
extern void C_PS();


CProc graphicsTable[] = {
  
  /*
   * CProcs in gobject.c 
   */      
  "window",C_Window,"{{{list [<regexp>='*'] } {Gets the list of all the windows whose name matches <regexp>}} \
{{new [<class> = 'Window'] <name> [list of fields...]} {Creates a window of class <class> named <name> and sends all the [list of fields...] to the 'setg' command.}}}", 

  "gupdate",C_GUpdate,"{{{start} {When called, it creates a new 'update' table and makes it current (the old current update table will become active as soon as \
this new one is closed). Any gobjects whose fields change and who are not explicitely redrawn \
are saved in the current 'update' table. The current table is emptied when 'gupdate do' is called. At that time all the gobjects of this table are \
redrawn as well as all the gobjects in front of them and the ones in the back (if the background color is invisible). \
These commands must be used inside a script command (all the update tables are deleted when terminal is waiting for a command line to be typed \
in). Let us note that whenever a 'setgu' command is executed, a 'gupdate start' is executed before changing any field and a 'gupdate do' \
is executed at the end of the 'setgu' command. Warning : only gobjects belonging to the same window can be put in an update table.}} \
{{do} {Performs the current update. It redraws all the gobjects that needs to be updated since 'gupdate start' was called.}} \
{{add <gobjectlist>} {Adds a list of gobjects to the current update table so that these gobjects will be redrawn when 'gupdate do' is called.}}}",

  "msge",C_Msge,"{{{*gobjectRegexp* *msge* <arg1>...<argN>} {Sends a message to all the gobjects whose name matches \
*gobjectRegexp*. The message is *msge* and <arg1>...<argN> are the arguments to the message. If the message is \
unknown it returns '' otherwise it returns a value different from ''.}}}",\

  "setg",C_Setg,"{{{[-] *gobjectRegexp* -*field1* [..<val>..] ... -*fieldN* [..<val>..>]} {Sets/Gets the fields *field1*...*fieldN* \
of all the gobjects whose name matches *gobjectRegexp*. If no <val> are specified then just one field should be used and its value is returned. \
Otherwise, it sets the fields *field1*...*fieldN* with the respective values specified by the <val>s (a single field might need several \
<val> to be set). In any case 'setg' returns 0 if the field does not exist. If a field is changed then it can return either 1 or -1.  \
If it returns 1, it means that the object should be redrawn, thus, consequently, the corresponding gobject is stored in the \
current update table if any. \
If it returns -1, it means that the gobject should not be redrawn and thus it it is not stored in the current update table. \
In any case, if one wants to keep lastwave from storing the gobject in the current update table (i.e., from redrawing it later), \
one must \
insert a '-' before the object name. Let us note that one can send \
messages to the sons of any gobject that matches *gobjectRegexp* on the same command line by inserting in the command line the string \
'-*gobjectRegexp*' followed by a regular 'setg' list of argument. For instance, if you want to set the background color \
of any gobject inside the window 'win' to black and at the same time change the foreground color of the object 'win.view' to red, you would type \
\"setg win -..* -bg 'black' -.view -fg 'red'\"}}}",
 
   "setgu",C_SetgU,"{{{*gobjectRegexp* -*field1* [..<val>..] ... -*fieldN* [..<val>..>]} {Same as the 'setg' command but it updates the display of the corresponding gobjects. \
It is basically equivalent to perform successively : 1- 'gupdate start' \
2- a 'setg' and 3- a 'gupdate do'. Thus any gobject which has changed will be redrawn.}}}",

   "gclass",C_GClass,"{{{father <class> [<fatherClass>]} {Gets the name of the father class of the class <class> if <fatherClass> is not specified. \
Otherwise it tests whether <class> inherits from <fatherClass> and returns 1 (if it is) or 0 (if not)}} \
{{type <class>} {Gets the eventual variable type associated to the class 'class' (returns it or returns '').}} \
{{objlist (<class> | '*') *objlist*} {Gets a listv of the complete names of all the gobjects of class <class> (or of any class \
if '*' is used) which matches the pattern *objlist*.}} \
{{package <class>} {Gets the package name the class is defined in.}} \
{{help <class> (setg | msge)} {Gets the help listv on the 'setg' or 'msge' class procedure. It is a listv made of several listv (one for each field or each message) with 2 elements :\
 a usage and a one-line help.}} \
{{list [<nameRegexp>='*'] [<packageNameRegexp>='*'] [<&type>]} {Gets all the names of gclasses which name matches the <nameRegexp>, who belongs to \
a package whose name matches <packageNameRegexp> and who can display the content of <&type> variables (or of any type if <&type> is not specified).}} \
{{new *name* *fatherClass* (<setSCommand> | null) (<messageSCommand> | null) (<drawSCommand> | null) <info> [<isinSCommand>] [-t <&type>] [-lm]} {Creating a new \
gclass of name *name* based on the class *fatherClass*.  The arguments are :\n\
- <setSCommand> : A procedure that will be used to set/get the fields of the newly defined gobjects. \
Its argument must be 'obj field .l' where 'obj' is the gobject name, 'field' is the name of the field (excluding '-', e.g., 'pos' or 'size') \
and 'l' is a listv that groups whatever is left. If it is empty, it means that the procedure must return the value of the field (i.e., this corresponds to \
a 'get'), otherwise the procedure must set the field using the arguments in 'l'. If the field 'field' is not taken care by the \
procedure <setSCommand>, the procedure MUST not return any value. Let us note that in the case of a 'set', it must return either '1' if the gobject \
must be added to the current update list or '-1' if not. If the field name starts with '-?' (e.g., '-?bound' for Views or '-?wtrans' for \
GraphWtrans), then it means that \
this field is a 'read-only' field that can use the arguments in 'l'. Thus, the procedure should just return the field value. \n\
- <messageSCommand> : A procedure name that will be used to perform messages sent to the newly defined gobjects. Its argument \
must be 'obj msge .l' where 'obj' is the gobject name, 'msge' is the name of the msge and 'l' the listv of all the remaining arguments. \
If the message is accepted, the procedure <messageSCommand> should return something (1 for instance)  and it should return nothing if not. \
Let us note that there are 3 special messages that \
you can redirect : the 'init' message that is sent right after a new gobject of class *name* is created ('l' is empty and this message is first sent \
to the gobject as an object of the class 'GObject' and so on to all the classes it inherits from, from top to bottom), \
the 'delete' message that is sent right before a gobject of class *name* is deleted, ('l' is empty, and it is sent to the gobject as an object of all \
the gclasses it inherits from, from bottom to top) and the 'deleteNotify' message that is sent to a 'GList' (and all the classes that \
inherit from GList) whenever one \
of its gobject is asked to be deleted, the name of the gobject is 'l' and it must return 1 if it accepts that the object is deleted \
or 0 to forbid deletion. \n\
- <drawSCommand> : A procedure name that will be used to draw the newly defined gobjects. \
Its argument must be 'obj .l' where 'obj' is the gobject name and 'l' is a listv of 4 floats representing the rectangle {x y w h} that \
must be redrawn (using local coordinates). The clip rectangle is automatically set to this rectangle before the procedure is called, \
thus you should not worry redrawing more than what is asked if it is easier to manage. \n\
- <isinSCommand> : An optional procedure name that will be used to check whether the mouse is in one of the newly defined gobjects (if \
it is not specified then the mouse will be considered to be in the gobject if it is in the rectangle given by '-pos' and '-size'). \
Its argument must be 'obj x y' where 'obj' is the gobject name and 'x' and 'y' the local coordinates of the mouse. \
It must return a strictly negative number if the mouse is not in the gobject and otherwise a positive number which quantifies the distance \
between the mouse and the gobject. Lastwave always sends mouse events to the 'closest' gobject. If it returns 0, then lastwave will not try \
any other gobject and will consider this gobject to be the closest. \n\
- <info> : This is just a one-line help to describe what the class is meant for \n\
-m : If set then the gobjects of the newly defined class cannot be moved or resized. \n\
-l : If set then the system of local coordinate system of the gobjects of the newly defined class will be the same one as the father\\'s system \
(this is the case of GraphSignal for instance). \n\
-t : If set, the newly defined class is meant to display the content of variables of type <&type>. In that case, the <setSCommand> procedure of the class \
should know about the following fields : \
'-graph <value>' to load the value <value> of type <&type> into the gobject,  \
'-cgraph <value>' same as '-graph' but copies the value first (unless it is really too big in which case it just does \
the same thing as '-graph') and '-?<type> <variable>' (such as '-?signal' or '-?wtrans') to get the value that is currently \
displayed in a gobject and put it into the variable <variable>.}}}",
    
  /*
   * CProcs in window_color.c 
   */  
  "color",C_Color,"{{{nnew <name> (rgb <r> <g> <b> | hsv <h> <s> <v>)} {Creates a new named color using either RGB or HSV convention.}} \
{{inew [<colorMap>=<currentColorMap>] <index> (rgb <r> <g> <b> | hsv <h> <s> <v>)} {Creates a new indexed color of a colormap using either RGB or HSV convention.}} \
{{ilist [<colormap>=<currentColorMap>] [<index>]} {Gets a rgb definition listv '{r g b}' of the color <index> in <colormap> or (if <index> is not specified) a listv of all the colors {r g b}.}} \
{{nlist [<nameRegexp>='*']} {Gets a listv of the rgb definition '{name r g b}' of colors whose name matches <nameRegexp>.}} \
{{install} {Install all the colors that were defined with the 'color' command.}} \
{{animate <color> (rgb <r> <g> <b> | hsv <h> <s> <v>)} {Changes interactively the definition of the color <color> using RGB or HSV convention (Warning : It only works on Unix computers).}} \
{{nb} {Returns the total number of available colors for lastWave.}}}",
  "setcolor",C_SetColor,"{{{[-bg <color>] [-fg <color>] [-mouse [<color>]]} {Sets any (or all)  of the background (-bg), foreground (-fg) or mouse (used for inverse mode on Unix computers only) color.}}}",
  "colormap",C_ColorMap,"{{{new [<colorMap>=<currentColorMap>] <nbOfColors>} {Creates/Modifies the <colorMap> so that it can store <nbOfColors> colors.}} \
{{current [<colorMap>]} {Gets/Sets the current color map.}} \
{{size [<colorMap>=<currentColorMap>]} {Gets the size of <colorMap>.}} \
{{delete [<colorMap>=<currentColorMap>]} {Deletes the <colorMap>.}} \
{{list [<nameRegexp>='*']} {Gets a list of all the matching color map names.}}}",

  /*
   * CProcs in window_manager.c 
   */      
   "draw",C_Draw,"{{{lineto *gobject* <x> <y>  [-clip] [-pen <penSize>] [-color <color>] [-line ('dash' | 'solid')] [-mode ('normal' | 'inverse')]} \
{Draws in the *gobject* a line from the last point that has been drawn to <x> <y>\n\
-clip : The clip rectangle is set to be the *gobject* rectangle. \n\
-color : The color <color> is used for drawing the line. \n\
-line : If 'dash' then the line will be dashed \n\
-mode : If 'inverse' then the drawing uses the inverse color mode so that if the same drawing command is executed a second time, the line will disappear. \n\
-pen : The size of the pen <penSize> is used for drawing the line}} \
{{line *gobject* <x1> <y1> <x2> <y2> [-clip] [-pen <penSize>] [-color <color>] [-line ('dash' | 'solid')] [-mode ('normal' | 'inverse')]} \
{Draws in the *gobject* a line from the point <x1>,<y1> to <x2>,<y2> \n\
-clip : The clip rectangle is set to be the *gobject* rectangle. \n\
-color : The color <color> is used for drawing the line. \n\
-line : If 'dash' then the line will be dashed \n\
-mode : If 'inverse' then the drawing uses the inverse color mode so that if the same drawing command is executed a second time, the line will disappear. \n\
-pen : The size of the pen <penSize> is used for drawing the line.}} \
{{ellipse *gobject* <x> <y> <w> <h> [-clip] [-pen <penSize>] [-color <color>] [-line ('dash' | 'solid')] [-mode ('normal' | 'inverse')] [-pixel] [-rectType <rectType>] \
[-centered] [-fill]} \
{Draws in the *gobject* an ellipse in the rectangle <x> <y> <w> <h>. \n\
-centered : The ellipse is centered at <x>,<y> and the respective radii are <w> and <h>. \n\
-clip : The clip rectangle is set to be the *gobject* rectangle. \n\
-color : The color <color> is used for drawing (and filling) the ellipse. \n\
-fill : The ellipse is filled with the current color. \n\
-rectType : Changes the rectangle type that is used. Read the manual pages about  the 'rectType' field of the gobjects. \n\
-line : If 'dash' then the contour of the ellipse will be drawn using dashed lines \n\
-mode : If 'inverse' then the drawing uses the inverse color mode so that if the same drawing command is executed a second time, the ellipse will disappear. \n\
-pen : The size of the pen <penSize> is used for drawing the ellipse. \n\
-pixel : The sizes <w> and <h> are given in pixels instead of local coordinates.}} \
{{rect *gobject* <x> <y> <w> <h> [-clip] [-pen <penSize>] [-color <color>] [-line ('dash' | 'solid')] [-mode ('normal' | 'inverse')] [-pixel] [-rectType <rectType>] \
[-centered] [-fill]} \
{Draws in the *gobject* the rectangle <x> <y> <w> <h>. \n\
-centered : The rectangle is centered at <x>,<y> and the respective radii are <w> and <h>. \n\
-clip : The clip rectangle is set to be the *gobject* rectangle. \n\
-color : The color <color> is used for drawing (and filling) the rectangle. \n\
-fill : The rectangle is filled with the current color. \n\
-rectType : Changes the rectangle type that is used. Read the manual pages about  the 'rectType' field of the gobjects. \n\
-line : If 'dash' then the contour of the rectangle will be drawn using dashed lines \n\
-mode : If 'inverse' then the drawing uses the inverse color mode so that if the same drawing command is executed a second time, the rectangle will disappear. \n\
-pen : The size of the pen <penSize> is used for drawing the rectangle. \n\
-pixel : The sizes <w> and <h> are given in pixels instead of local coordinates.}} \
{{cross *gobject* <x> <y> <r>  [-clip] [-pen <penSize>] [-color <color>] [-line ('dash' | 'solid')] [-mode ('normal' | 'inverse')]} \
{Draws in the *gobject* a cross centered at <x> <y> and with a pixel radius of <r>\n\
-clip : The clip rectangle is set to be the *gobject* rectangle. \n\
-color : The color <color> is used for drawing the cross. \n\
-line : If 'dash' then the cross will be dashed. \n\
-mode : If 'inverse' then the drawing uses the inverse color mode so that if the same drawing command is executed a second time, the cross will disappear. \n\
-pen : The size of the pen <penSize> is used for drawing the cross}} \
{{point *gobject* <x> <y> [-clip] [-color <color>] [-pen <pensize>] [-mode ('normal' | 'inverse')]} \
{Draws in the *gobject* the point <x> <y> \n\
-clip : The clip rectangle is set to be the *gobject* rectangle. \n\
-color : The color <color> is used for drawing the point. \n\
-mode : If 'inverse' then the drawing uses the inverse color mode so that if the same drawing command is executed a second time, the point will disappear. \n\
-pen : The size of the pen <penSize> is used for drawing the point.}} \
{{string *gobject* <str> [*hPositionMode*=left] <x> [*vPositionMode*=base] <y> [-clip] [-color <color>] [-mode ('normal' | 'inverse')] [-font <fontName>]} \
{Draws the (eventually multiple line) string <str> at the position <x> <y> using the gobject font. The two parameters <hPositionMode> and <vPositionMode> \
fix how these coordinates are used. \n\
if <hPositionMode>=='left' then the string is justified to the left and <x> corresponds to its left position. \n\
if <hPositionMode>=='rightN' then each individual line of the string is justified to the right and <x> corresponds to their right positions. \n\
if <hPositionMode>=='right1' then all the lines (as a group) of the string are justified to the right and <x> corresponds to its right position. \n\
if <hPositionMode>=='middleN' then each individual line of the string is justified to its middle point and <x> corresponds to their middle positions. \n\
if <hPositionMode>=='middle1' then all the lines (as a group) of the string is justified to its middle point and <x> corresponds to its middle position. \n\
if <vPositionMode>=='base' then <y> corresponds to the position of the baseline of the first line. \n\
if <vPositionMode>=='down' then <y> corresponds to the position of the bottom of the last line. \n\
if <vPositionMode>=='up' then <y> corresponds to the position of the top of the first line. \n\
if <vPositionMode>=='middle' then <y> corresponds to the position of the middle of the whole string \n\
if <vPositionMode>=='middleup' in case <str> has just one line, <y> corresponds to the position of the middle of the characters above the baseline (such as '1') \n\
-clip : The clip rectangle is set to be the *gobject* rectangle. \n\
-color : The color <color> is used for drawing the string. \n\
-mode : If 'inverse' then the drawing uses the inverse color mode so that if the same drawing command is executed a second time, the string will disappear. \n\
-font : Uses a specified font \
-pen : The size of the pen <penSize> is used for drawing the string.}} \
{{axis *gobject* <x> <y> <w> <h> <xMin> <xMax> <yMin> <yMax> [-title <title>] [-xlabel <label>] [-ylabel <label>] [-margin <nbOfPixels>] [-font] [-clip] [-pen <penSize>] [-color <color>] [-line ('dash' | 'solid')] [-mode ('normal' | 'inverse')] [-reverse (x | y | xy | none)]} \
{Draws x and y-axis around the rectangle <x>, <y>, <w>, <h>. \
These axes correspond to abscissa ranging from <xMin> to <xMax> and ordinate ranging from <yMin> to <yMax> \n\
-reverse : The state that indicates which axis are reversed compared to the regular window coordinate system (y-axis going \
from top to bottom and x-axis going from left to right). Let us note that the same field exists for Views, so that they can be used along with axis. "
"If it is equal to 'y' (the default value at initialization) then the y-axis will be going from bottom of the window to the top, if it is equal to 'x' the x-axis \
will be going from right to left, if it is equal to 'xy' both will be combined and if it is equal to 'none' the y-axis will be top to bottom and the x-axis left to right \
(as for windows and regular glists). \n\
-frame : If not set then only two axis are drawn. If set then the whole rectangle <x>,<y>, <w>, <h> is surrounded by axes (i.e., 4 axes are drawn). \n\
-ticksIn : The ticks on the axis are inside the rectangle instead of outside. \n\
-margin : A margin of size <nbOfPixels> is used around the rectangle <x>, <y>, <w>, <h> to draw each axis. \n\
-clip : The clip rectangle is set to be the *gobject* rectangle. \n\
-color : The color <color> is used for drawing the axis. \n\
-pen : The size of the pen <penSize> is used for drawing the axis. \n\
-font : Uses a specified font}} \
{{gobject *glist* *gclass* [-clip] [list of -<fieldN> [<val1>...<valP>]]} {Draws (without creating it) a gobject of class *gclass* in the <glist> and whose  fields are specified \
by the -<fieldN> [<val1>...<valP>] list. If '-clip' then the current clip rectangle is not used and the entire object is drawn otherwise the current \
clip rectangle is used.}}}",

   "font",C_Font,"{{{info <font>} {Gets an information list about the font. The list is made of 3 numbers : <ascent> <descent> and \
<interline>. <ascent> is the maximum of points (for all characters of the font) above the base line (excluding \
the base line), <descent> is the maximum of points (for all characters of the font) below the base line (including \
the base line) and <interline> is such that 2 base lines of 2 successive lines are vertically spaced by the number \
of points : <ascent>+<descent>+<interline>.}} \
{{exist <fontName>} {Returns 1 if the font named <fontName> exists and 0 if not. The name of the font \
is of the form <name>-<size>-<style> (e.g., Geneva-9-plain) where <style> is either 'plain', 'bold', 'italic', 'boldItalic'.}} \
{{default [<fontName>]} {Gets/Sets the font that is used by default by all the gobjects.}} \
{{size <font>} {Gets the size of the <font>}} \
{{style <font>} {Gets the style of the <font>}} \
{{name <font>} {Gets the basic name (e.g. Geneva, Times...) of the <font>}} \
{{list [<nameRegexp>='*'] [<size>='*'] [<styleRegexp>='*']} {Gets a list (not a listv!!) of the available fonts matching the pattern \
<name>-<size>-<style> where <name> and <style> are pattern matching expressions and <size> is either a number \
or '*'. WARNING : on Macintosh computers, the returned list of fonts is a simple list of \
names since all the styles and sizes are available for true type fonts. Moreover, this list is not complete \
(it will be in a future version of LastWave). On Unix/X11 computers, \
the list is a list of regular font names of the type <name>-<size>-<style>. However there can be duplicates.}} \
{{rect <font> <string> <hPositionMode> <x> <vPositionMode> <y> [*gobject*]} {Gets a listv representing the bounding rectangle of the string <string> \
if it were drawn using <font> and using the 'draw string' command in the *gobject* (or any window if *gobject* is not specified) \
along with the arguments <hPositionMode>, <x>, <vPositionMode> and <y>. Read the 'draw string' manual pages to \
learn about these arguments.}}}",

  /*
   * CProcs in event.c 
   */      
  "setbinding",C_SetBinding,"{{{<bindingGroupName> *gclass* ((left | middle | right)Button('' | Up | Down | Motion) | motion | leave | enter | enterLeave | key | keyUp | keyDown | draw | delete | error) ['key1 .. keyN'] [<modifiers>] <script>} \
{Defines event bindings associated to gobjects of class *gclass*. Read the manual pages for examples of bindings.}}}",
  "binding",C_Binding,"{{{delete <groupNameRegexp> [*class*]} {Deletes all the bindings that correspond to all binding groups whose name matches <groupNameRegexp>.}} \
{{activate <groupNameRegexp> [*class*]} {Activates all the bindings that correspond to all binding groups whose name matches <groupNameRegexp>.}} \
{{deactivate <groupNameRegexp> [*class*]} {Deactivates all the bindings that correspond to all binding groups whose name matches <groupNameRegexp>.}} \
{{info <groupNameRegexp> [*class*]} {Get info on the bindings that correspond to a binding group which matches <groupNameRegexp>.}}}",
  "event",C_Event,"{{{process [<time>=0]} {Waits for <time> seconds while processing to events. If <time>==0 then \
it processes all the events in the queue and then returns}}}", 


  /* 
   * In postscript.c.c 
   */
  "ps", C_PS,"{{{csize [<cmXSize> [<cmYSize>=-1]]} {If no argument, it returns a listv made of the x and y sizes of the postscript drawing in centimeters. \
If both arguments are specified, it sets the x and y sizes (centimeters). And if only the first argument is specified (second is -1), \
it sets the x-size and the y-size will be automatically computed using the x/y size proportion of the window to draw.}} \
{{isize [<inchXSize> [<inchYSize>=-1]]} {If no argument, it returns a listv made of the x and y sizes of the postscript drawing in inches. \
If both arguments are specified, it sets the x and y sizes (inches). And if only the first argument is specified (second is -1), \
it sets the x-size and the y-size will be automatically computed using the x/y size proportion of the window to draw.}} \
{{cpos [<cmXPos> <cmYPos>]} {Sets/Gets the x and y distances (in centimeters) of the postscript drawing from the top-left \
corner of the paper. Warning : This is not quite working yet.}} \
{{linewidth [<linewidth>]} {Sets/Gets the linewidth used for postscript. A width of 1 is an average linewidth.}} \
{{ipos [<inchXPos> <inchYPos>]} {Sets/Gets the x and y distances (in inches) of the postscript drawing from the top-left \
corner of the paper. Warning : This is not quite working yet.}}}",
  

  /*
   * CProcs in macwindow.c or X11window.c 
   */      
  "system",C_System,"{{{mouse1} {Declares that the mouse is a 1-button mouse (For Macintosh computers only).}} \
{{mouse3} {Declares that the mouse is a 3-button mouse (For Macintosh computers only).}} \
{{forceKeyUp} {Declares that the keyboard does not send keyup events (For Macintosh computers only).}} \
{{noForceKeyUp} {Declares that the keyboard does send keyup events (For Macintosh computers only).}} \
{{nEvents} {Gets the total number of events that were managed up to the current time.}} \
{{resizeDelay [<delay>]} {Sets/Gets the minimum delay between to resize events (so that there are not too many!) (For Windows/Unix computers only).}}}",

  NULL,NULL,NULL        
};



/* The ccommand tables of the kernel */
CProcTable cprocTables[] = {

  interpreterTable,"kernel","Basic interpreter commands",
  graphicsTable,"kernel","Basic graphic commands",

  NULL,NULL,NULL
};



