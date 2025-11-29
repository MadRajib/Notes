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
- Trick

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