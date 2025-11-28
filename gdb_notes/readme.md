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