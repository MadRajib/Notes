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

TUI: Text User Interface

```bash
crtl + x + a : -> Toggle to/enable TUI enale
```