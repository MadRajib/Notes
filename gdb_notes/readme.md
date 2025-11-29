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

### Tricks

```bash
$ gdb a.out
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

### Reverse Debugging
