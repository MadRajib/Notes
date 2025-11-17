## Pointers
### Pointer Comparison

* Two pointer values are equal if they point to the same location, or if they are both null.
* Ordering comparisons such as > and >= operate on pointers by converting them to unsigned integers.
* The C standard says the two pointers must point within the same object in memory, but on GNU/Linux systems these operations simply compare the numeric values of the pointers.

* The pointer values to be compared should in principle have the same type, but they are allowed to differ in limited cases. First of all, if the two pointers’ target types are nearly compatible, the comparison is allowed.
* If one of the operands is void * (see Void Pointers) and the other is another pointer type, the comparison operator converts the void * pointer to the other type so as to compare them. (Not allowed in standard C)
* Comparison operators also allow comparing the integer 0 with a pointer value. This works by converting 0 to a null pointer of the same type as the other operand.

### Pointer Arithmetic

* Adding an integer (positive or negative) to a pointer is valid in C.
* It assumes that the pointer points an element in an array.

```c
int *p = &array[0];
elt0 = *p;  // fetch 1st element

++p;        // jmp to next element increments as size of int
```

* Adding an integer to a pointer moves it by that many array elements.
* Subtracting two pointers gives you back that same integer distance
```c
int *p0, *p3, *p4;

int *p = &array[4]; // p points to the 4th element
p4 = p; 

--p; // points to 3rd elem
p3 = p;

p -= 3; // points to 0 elem
p0 = p;

printf ("%d, %d, %d, %d\n",
          p4 - p0, p0 - p0, p3 - p0, p0 - p3);
  /* Prints 4, 0, 3, -3.  */
```

* The addition operation does not know where arrays begin or end in memory.
* All it does is add the integer __(multiplied by target object size)__ to the numeric value of the pointer.
* C defines pointer subtraction to be consistent with pointer-integer addition, so that (p3 - p1) + p1 equals p3, as in ordinary algebra. Pointer subtraction works by subtracting p1’s numeric value from p3’s, and dividing by target object size. 

__Note:__

* In standard C, addition and subtraction are not allowed on void *, since the target type’s size is not defined in that case.
* Likewise, they are not allowed on pointers to function types.
* However, these operations work in GNU C, and the “size of the target type” is taken as 1 byte.

### Compatible types
* what it means in C for two types to be compatible.
* Declaring a function or variable twice is valid in C only if the two declarations specify compatible types.

Valid:
```c
/*---- Variable declartions ------ */

// In File1.c (The Definition)
int global_count = 100;

// In File2.c (A compatible declaration)
extern int global_count; // Valid: 'extern int' is compatible with 'int'

// Also in File2.c (Another compatible declaration)
extern signed int global_count; // Still valid: 'signed int' is compatible with 'int'

// In a header file (MyMath.h)
// Declaration 1: Prototype with parameter names
int add(int a, int b);

/*---- functions ------ */

// In a C file (MyMath.c)
// Declaration 2: The actual function definition
int add(int a, int b) {
    return a + b;
}

// In another C file (Main.c)
#include "MyMath.h"

// Declaration 3: Another (redundant) compatible declaration
// This is valid because 'int(int, int)' is compatible with 'int(int a, int b)'
extern int add(int, int);

int main() {
    int result = add(5, 10); // All declarations link to the same function
    return 0;
}
```
Invalid:
```c
int my_value;
float my_value; // ERROR: Conflicting types for 'my_value'
                // 'int' is not compatible with 'float'

char* user_name;
int* user_name; // ERROR: Conflicting types for 'user_name'
               // 'char*' is not compatible with 'int*'

// Declaration 1
int calculate(int x);

// Declaration 2
float calculate(int x); // ERROR: Conflicting types for 'calculate'
                        // The return type 'float' is not compatible with 'int'

// Declaration 1
int process(int data);

// Declaration 2
int process(char data); // ERROR: Conflicting types for 'process'
                        // The parameter type 'char' is not compatible with 'int'
```