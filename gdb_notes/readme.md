## GDB

Invoking GDB
```bash
$ gdb program

// with a core file
$ gdb program core 

// debug a running process
$ gdb program 1234
$ gdb -p 1234

// run a.out and pass argument
// method 1
$ gdb --args a.out arg1 arg2

// method 2
$ gdb a.out
(gdb) set args arg1 arg2 arg3

// method 3
$ gdb a.out
(gdb) run arg1 arg2 arg3

// check current args
(gdb) show args

// set environment variables
(gdb) set environment VAR=value
(gdb) run

// without printing the fron material
$ gdb --silent

$ gdb -help
```
Modify variables
- NOTE: in newer versions of gdb, it may be necessary to use the command 'set var', as in 'set var x = 3'

```bash
(gdb) set x = 3
(gdb) print x
$4 = 3
```
Call functions linked into the program
```bash
(gdb) call abort()
```
Return from a function
- Use the finish command to have a function finish executing and return to it's caller
```bash
(gdb) finish
Run till exit from #0  fun1 () at test.c:5
main (argc=1, argv=0xbffffaf4) at test.c:17
17        return 0;
Value returned is $1 = 1
```
To get backtrace
- Use the gdb command backtrace. In the backtrace below, we can see that we are currently inside func2(), which was called bu func1(), which was called from main()
```bash
(gdb) backtrace
#0  func2 (x=30) at test.c:5
#1  0x80483e6 in func1 (a=30) at test.c:10
#2  0x8048414 in main (argc=1, argv=0xbffffaf4) at test.c:19
#3  0x40037f5c in __libc_start_main () from /lib/libc.so.6
(gdb)
```
change stack frames?
- Use the gdb command frame. Notice in the backtrace above that each frame has a number beside it. Pass the number of the frame you want as an argument to the command.
```bash
(gdb) frame 2
#2  0x8048414 in main (argc=1, argv=0xbffffaf4) at test.c:19
19        x = func1(x);
(gdb)
```
examine stack frames?
- To look at the contents of the current frame, there are 3 useful gdb commands. info frame displays information about the current stack frame. info locals displays the list of local variables and their values for the current stack frame, and info args displays the list of arguments.
```bash
(gdb) info frame
Stack level 2, frame at 0xbffffa8c:
 eip = 0x8048414 in main (test.c:19); saved eip 0x40037f5c
 called by frame at 0xbffffac8, caller of frame at 0xbffffa5c
 source language c.
 Arglist at 0xbffffa8c, args: argc=1, argv=0xbffffaf4
 Locals at 0xbffffa8c, Previous frame's sp is 0x0
 Saved registers:
 ebp at 0xbffffa8c, eip at 0xbffffa90

(gdb) info locals
x = 30
s = 0x8048484 "Hello World!\n"

(gdb) info args
argc = 1
argv = (char **) 0xbffffaf4
```

Set break points
```bash
(gdb) break 19
Breakpoint 1 at 0x80483f8: file test.c, line 19

(gdb) break test.c:19
Breakpoint 2 at 0x80483f8: file test.c, line 19

(gdb) break func1
Breakpoint 3 at 0x80483ca: file test.c, line 10

// break point on a cpp function
(gdb) break TestClass::testFunc(int) 
Breakpoint 1 at 0x80485b2: file cpptest.cpp, line 16.
```

How to set a temporary breakpoint ?
- Use the tbreak command instead of break. A temporary breakpoint only stops the program once, and is then removed.

List all breakpoints
```bash
(gdb) info breakpoints
Num Type           Disp Enb Address    What
2   breakpoint     keep y   0x080483c3 in func2 at test.c:5
3   breakpoint     keep y   0x080483da in func1 at test.c:10
```
Disable Breakpoints
```bash
(gdb) disable 2
(gdb) info breakpoints
Num Type           Disp Enb Address    What
2   breakpoint     keep n   0x080483c3 in func2 at test.c:5
3   breakpoint     keep y   0x080483da in func1 at test.c:10
```

Skip Breakpoints
- The ignore command takes two arguments: the breakpoint number to skip, and the number of times to skip it.
```bash
(gdb) ignore 2 5
Will ignore next 5 crossings of breakpoint 2.
```

Set a write watchpoint for a variable:
```bash
(gdb) watch x
Hardware watchpoint 4: x
(gdb) c
Continuing.
Hardware watchpoint 4: x

Old value = -1073743192
New value = 11
main (argc=1, argv=0xbffffaf4) at test.c:10
10      return 0;
```
set a read watchpoint for a variable:
```bash
(gdb) rwatch y 
Hardware read watchpoint 4: y
(gdb) continue
Continuing.
Hardware read watchpoint 4: y

Value = 1073792976
main (argc=1, argv=0xbffffaf4) at test.c:8
8         x = y;
```
Set a read/write watchpoint for a variable:
- Use the awatch command. Usage is identical to the watch command.

Disable Watchpoints
- Active watchpoints show up the breakpoint list.
- Use the info breakpoints command to get this list. Then use the disable command to turn off a watchpoint, just like disabling a breakpoint.
```bash
(gdb) info breakpoints
Num Type           Disp Enb Address    What
1   breakpoint     keep y   0x080483c6 in main at test.c:5
        breakpoint already hit 1 time
4   hw watchpoint  keep y   x
        breakpoint already hit 1 time
(gdb) disable 4
```

Examine memory
- The syntax for the x command is x/<count><format><size> <address>.
- *count* → how many units to display
- *size* → how many bytes per unit
    - b -> bytes; h -> half-word; w -> word; g -> giant word
- Format specifiers:
    - x -> hex; d -> signed dec; u -> unsigned dec; t -> binary
    - o -> octale; c -> character; f -> float; s -> C-string; i -> machine instr
```bash
Bytes in hex:
(gdb) x/8xb 0x7fffffffe000
0x7fffffffe000: 0x55 0x48 0x89 0xe5 0x48 0x83 0xec 0x10

Words (4 bytes) in hex:
(gdb) x/4xw 0x7fffffffe000
0x7fffffffe000: 0xe5894855 0x10ec8348 0x24448948 0x89480000

Giant words (8 bytes) in hex:
(gdb) x/2xg 0x7fffffffe000
0x7fffffffe000: 0x10ec8348e5894855 0x0000000000444889

Signed decimal:
(gdb) x/4dw 0x7fffffffe000
0x7fffffffe000: -1770702563 283997384 11223344 55667788

Unsigned decimal:
(gdb) x/4uw 0x7fffffffe000
0x7fffffffe000: 2524264733 283997384 11223344 55667788

Binary:
(gdb) x/4tb 0x7fffffffe000
0x7fffffffe000: 01010101 01001000 10001001 11100101

Characters:
(gdb) x/10cb 0x7fffffffe040
0x7fffffffe040: 'H' 'e' 'l' 'l' 'o' ' ' 'W' 'o' 'r' 'l'

C-string:
(gdb) x/s 0x7fffffffe040
0x7fffffffe040: "Hello World"

Assembly instructions:
(gdb) x/5i $rip
0x400540: push rbp
0x400541: mov  rbp, rsp
0x400544: sub  rsp, 0x10
0x400548: mov  DWORD PTR [rbp-0x4], edi
0x40054b: mov  QWORD PTR [rbp-0x10], rsi

Floats:
(gdb) x/4fw 0x7fffffffe000
0x7fffffffe000: 3.14 2.71 1.41 0.57

Array dump (symbol name):
(gdb) x/5dw arr
0x601040 <arr>: 10 20 30 40 50

Pointer:
(gdb) x/x $rbp
0x7fffffffe0a0: 0x7fffffffe0c0

Pointer dereference:
(gdb) x/x *$rbp
0x7fffffffe0c0: 0x0000000000000042

Hex dump (64 bytes):
(gdb) x/64bx 0x7fffffffe000
0x7fffffffe000: 0x55 0x48 0x89 0xe5 0x48 0x83 0xec 0x10 ...

Word dump (32 words):
(gdb) x/32wx 0x7fffffffe000
0x7fffffffe000: 0xe5894855 0x10ec8348 ...
```
Step through the code at the instruction level
- There are two commands, nexti and stepi, that work similar to next and step.

See the assembly code my program is running
```bash
(gdb) disassemble main
Dump of assembler code for function main:
0x80483c0 <main>:       push   %ebp
0x80483c1 <main+1>:     mov    %esp,%ebp
0x80483c3 <main+3>:     sub    $0x18,%esp
0x80483c6 <main+6>:     movl   $0x0,0xfffffffc(%ebp)
0x80483cd <main+13>:    mov    0xfffffffc(%ebp),%eax
0x80483d0 <main+16>:    movb   $0x7,(%eax)
0x80483d3 <main+19>:    xor    %eax,%eax
0x80483d5 <main+21>:    jmp    0x80483d7 <main+23>
0x80483d7 <main+23>:    leave  
0x80483d8 <main+24>:    ret    
End of assembler dump.

(gdb) set disassembly-flavor intel

(gdb) set disassembly-flavor att

```

### Tricks

```bash
$ gdb a.out

// ~/.gdbinit
(gdb) set history save on
(gdb) set print pretty on
(gdb) set pagination off
(gdb) set confirm off
...

(gdb) break main
(gdb) run
(gdb) r -a /
or

(gdb) start    // Runs the exectuable and puts a break in main
```

To jmp to next line

```bash
(gdb) next
```
To get more context 
```bash
(gdb) list
1       #include <stdio.h>
2
3       int main(int argc, char **argv) {
4
5               int a;
6               int b;
7
8               printf("hello world\n");
9
10              return 0;
```

TUI: Text User Interface

```bash
(gdb) foucs cmd // focus set to cmd window
“ctrl-x,o”  is another way to change the focus between the different tui sections (“o” as in “other”)
```

```bash
crtl + x + a : -> Toggle to/enable TUI enale
or 
(gdb) tui eanble

ctrl + l -> Refresh the screen

ctrl + n -> next cmd
ctrl + p -> previous cmd

ctrl + x + 2 -> create split window
ctrl + x  + 2 (again) -> to toggle between the two window

```

- Can run python script inside
```bash
(gdb) python print('hello world')
hello world

(gdb) python
> import os
> end               // to exit
(gdb)
```
- can run shell commands
```bash
(gdb) shell ps
(gdb) shell ls
```
- python is not runned by forking a new process but with gdb itself.
- you can see the pid printed and ps are the same
```bash
(gdb) python
>import os
>print(os.getpid())
>
>end
2109
hi
(gdb) shell ps
    PID TTY          TIME CMD
    287 pts/0    00:00:00 bash
   2109 pts/0    00:00:01 gdb
   2127 pts/0    00:00:00 a.out
   2210 pts/0    00:00:00 ps
(gdb)
```
- python can interact with gdb
```bash
(gdb) python gdb.execute() // to do gdb commands
(gdb) python gdb.execute('next')
hello world
10              return 0;
(gdb)

(gdb) python gdb.parse_and_eval() // to get data from inferior
(gdb) python help('gdb')  // to see online help

// Setting breakpoints

(gdb) python bp = gdb.Breakpoint('test.c:2')

// Disable Breakpoint
(gdb) python bp.enabled = False

// Get all the breakpoints
(gdb) python bps = gdb.breakpoints()
(gdb) python
>print(bps)
>end
(<gdb.Breakpoint number=2 hits=1>, <gdb.Breakpoint disabled number=3 hits=0>)

// get breakpoint locations 
(gdb) python print(bps[0].location)
main

// print varaibles
(gdb) print a
$1 = 0
(gdb) python var_a = gdb.parse_and_eval('a')
(gdb) python print(var_a)
0
// if a is increments, ruing the above print(var_a) will print old value
```
- to get python gdb help
```bash
(gdb) python help(gdb)
```
- To print structures

```bash
(gdb) p st
$4 = {id = 123, name = 0x5555555592a0 "jon doe", dob = {tm_sec = 0, tm_min = 30, tm_hour = 9, tm_mday = 2, tm_mon = 9, tm_year = 100, tm_wday = 0,
    tm_yday = 0, tm_isdst = 0, tm_gmtoff = 0, tm_zone = 0x0}}

// set printing structures some what good
(gdb) set print pretty on

(gdb) p st
$5 = {
  id = 123,
  name = 0x5555555592a0 "jon doe",
  dob = {
    tm_sec = 0,
    tm_min = 30,
    tm_hour = 9,
    tm_mday = 2,
    tm_mon = 9,
    tm_year = 100,
    tm_wday = 0,
    tm_yday = 0,
    tm_isdst = 0,
    tm_gmtoff = 0,
    tm_zone = 0x0
  }
}
```
- to call a function
```c
struct student {
        unsigned id;
        char *name;
        struct tm dob;
};

static void
dump_student (struct student *st)
{
        const char *dob = asctime(&st->dob);
        printf("id: %u\nname: %s\ndob: (%p) %s\n",
                st->id, st->name, dob, dob);
}

int
main (void)
{
        struct student st = {
                .id = 123,
                .name = strdup("jon doe"),
                .dob = {
                        .tm_year = 100,
                        .tm_mon = 9,
                        .tm_mday = 2,
                        .tm_hour = 9,
                        .tm_min = 30,
                        .tm_sec = 0}
        };

        dump_student(&st);
```
```bash
(gdb) call dump_student(&st)
id: 123
name: jon doe
dob: (0x7ffff7fb02a0) Sun Oct  2 09:30:00 2000
```
- To use Python pretty print
```bash
(gdb) shell cat pretty.py 
```
```python
import time

class StudentPrinter(object):
    "Print a struct student."

    def __init__(self, val):
        self.val = val

    def to_string(self):
        print("month = %d" % self.val['dob']['tm_mon'])
        return ("id: %i name=%s dob=%s"
                % (self.val['id'], self.val['name'],
                   time.asctime(
                       (self.val['dob']['tm_year'] + 1900,
                        self.val['dob']['tm_mon'],
                        self.val['dob']['tm_mday'],
                        self.val['dob']['tm_hour'],
                        self.val['dob']['tm_min'],
                        self.val['dob']['tm_sec'],
                        self.val['dob']['tm_wday'],
                        self.val['dob']['tm_yday'],
                        self.val['dob']['tm_isdst']))))

    def display_hint(self):
        return 'student'


import gdb.printing
# create a pretty printer collection matching 'student' c pattern
pp = gdb.printing.RegexpCollectionPrettyPrinter('student')

# Adds your actual pretty-printer class to the collection.
# Arguments (in order):
#   'student' → Name of the printer (tag)
#   '^student$' → Regex that matches the type name
#       ^student$ means: match exactly the type "student" (no extra words).
#   StudentPrinter → Your Python class that formats the output.
pp.add_printer('student', '^student$', StudentPrinter)

# Register the pretty-printer for this debugging session so GDB uses it automatically whenever it prints student objects.
gdb.printing.register_pretty_printer(gdb.current_objfile(), pp)
```
- *gdb.printing* provides helper classes for registering and organizing pretty-printers so GDB can display custom C/C++ structures in a human-friendly format.

- using it
```bash
(gdb) source pretty.py
(gdb) print st
$2 = month = 9
id: 123 name=0x5555555592a0 "jon doe" dob=Mon Sep  2 09:30:00 2000
```

#### Custom Prompts
Default:
```bash
(gdb)
```
Static - use this to identfiy a particular GDB session:
```bash
set prompt <prompt>
show prompt
```
Dynamic - use Python to generate the prompt
```bash
import gdb
import time

def my_prompt_hook(current_propmt):
    my_prompt = time.strftime("%H:%M:%S")

    try:
        frame = gdb.selected_frame().name()
    except gdb.error:
        frame = "(no frame)"
    return f "[{now_str}] {frame} > "

# gdb will call this for every prompt
gdb.prompt_hook = my_prompt_hook
```
- Advance to line no
```bash
(gdb) advance 24
```
- set source window height
```bash
(gdb) winheight src 8
```
### Reverse Debugging

Load core file
```bash
$ gdb -c core.26529
```
```bash
(gdb) break main
(gdb) break _exit

// hook cmds when breakpoint is it
(gdb) command 2  // to run on breakpoint 2
>record
>continue
>end

(gdb) command 3
>run
>end

(gdb) r
// code will keep on runing after exiting untill it breaks

(gdb) bt
...
...

// step reverse in instructions
(gdb) reverse-stepi
(gdb) reverse-continue
(gdb) whatis array
type = long [32]  
```

### Remote server server
- Debug over serial/socket to a remote server
- Start
    ```bash
    $ gdbserver localhost:2000 ./a.out
    ```
- Then connect from a gdb with:
    ```bash
    (gdb) target remote localhost:2000
    ```

### Breakpoints

```bash
    tbreak                  // temporary breakpoint
    rbreak                  // break on regular expression match
    break foo thread 3      // stop at foo only in thread 3
    break foo if bar > 10   // stop at foo only if bar > 3

    delete [n]              // delete breakpoint number n
    delete [n]              // disable breakpoint number n
    enable [n]              // enable breakpoint number n
```
### Watchpoints
- watching an expression
```bash
watch foo               // stop when foo is modified
watch -l foo            // watch location
rwatch foo              // stop when foo is read
watch foo thread 3      // stop when thread 3 modifes foo
watch foo if foo > 10   // stop when foo is > 10

delete/disable/enable work for watchpoints too!
```
### Catchpoints
- Are like breakpoints but catch certain events, such as C++ exceptions.
```bash
catch catch             // to stop when c++ exception are caught
catch syscall nanosleep // to stop at nanosleep system call
catch syscall 100       // to stop at system call number 100

delete/disable/enable work for catchpoints too!
```
// Todo
### Multiple porcess debugging

- GDB calls each debugged program an inferior.
- Forks, execs, and multiple processes = multiple inferiors.

To Controls which process GDB follows after a fork().
- child → Debug the child process after fork
- parent → Stay with the parent process

```bash
(gdb) set follow-fork-mode child

If your program forks, GDB will switch to the child automatically
```
- By default, GDB detaches from the process it doesn’t follow
- Setting this to off keeps both processes under GDB control.

```bash
(gdb) set detach-on-fork off

Parent becomes inferior 1
Child becomes inferior 2
Both can be debugged
```
- To Show all inferiors currently loaded
```bash
(gdb) info inferiors
  Id   Target Id         Frame
* 1    process 2489      main ()
  2    process 2490      child_process ()
```

- To Swtich to infereior N

```bash
(gdb) inferior 2

Now GDB switches the context to process #2.
(gdb) bt
(gdb) p var
```
- To Control the behaviour after an exec() call
    - new → Treat exec() as a new inferior
    - same → Replace existing inferior’s program
```bash
(gdb) set follow-exec-mode new

```
- Manually add new inferiors (new debugged processes)
```bash
(gdb) add-inferior -exec ./worker

This creates a new inferior that can later be started:
(gdb) inferior 3
(gdb) run


Or duplicate the current one:
(gdb) add-inferior -copies 2

Makes 2 copies of the current inferior context.
```
- To Delete an inferior:
```bash
(gdb) remove-inferior 2

Process is removed from multiprocess list.
```
- To Duplicate the current inferior’s state, not the process.
```bash
(gdb) clone-inferior

Creates a new inferior with:
* Same executable
* Same breakpoints
* Same arguments
* Same environment
Used for multi-run analysis.
```

- To Print the current inferior ID
```bash
(gdb) print $_inferior
$1 = 2
```
### Non-stop mode
- Normally, when a thread hits a breakpoint, all threads stop.
- Non-stop mode changes this behaviour:
    - Only the stopped thread is halted
    - Other threads keep running
    - You can debug one thread while others continue executing
```bash
(gdb) set non-stop on
This must be done before running the program.

(gdb) continue -a

* -a = continue all threads
* Thread that hits breakpoint stops
* Other threads continue running
```
If pagination is on, GDB may freeze or behave unpredictably in non-stop mode.
```bash
(gdb) set pagination off
```
### Thread apply
### Calling inferior functions

### Dynamic Printf