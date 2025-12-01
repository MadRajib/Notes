## Extreme C

1. [Macros](#macros)

### Macros

Some of it uses:

- Defining a constant
- Using as a funtion instead of writing a C function
- loop unrolling
- header guards
= code generation
- conditional compilation
- and many more

```c
#define FOO 4   // defining

#undef FOO      // undefining

#define FOO(a, b) a + b // function like macro

error("Platfrom not defined!"); // throw error while preprocessing phase 
```
- modern C compilers are aware of C preprocessor directives. 
- they know about the source before entering the preprocessing phase.

- To see preprocessing result use __E__ option  with gcc or clang.
```bash
$ clang -E example.c
```
- *__Translation Unit__* ( or compilation uinit): is the preprocessed C code that is ready to be passed to the compiler.
    - In a TU all directives are substitued with inclusion or macro expansions and a flat long piece of C code has been produced.
- To define a new __domain specific language (DSL)__ and write code using it, is an important application of macros. eg
```c
#include <stdio.h>

#define PRINT(a) printf("%d\n", a);
#define LOOP(v, s, e) for (int v = s; v <= e; v++) {
#define ENDLOOP }

int main (int argc, char** argv) {
    LOOP(counter, 1, 10)
        PRINT(counter)
    ENDLOOP

    return 0;
}
```