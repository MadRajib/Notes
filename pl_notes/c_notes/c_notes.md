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
* In C, two different primitive types are never compatible.
* Likewise for the defined types struct, union and enum: two separately defined types are incompatible unless they are defined exactly the same way.


However, there are a few cases where different types can be compatible:
* Every enumeration type is compatible with some integer type. In GNU C, the choice of integer type depends on the largest enumeration value.
* Array types are compatible if the element types are compatible and the sizes (when specified) match.
* Pointer types are compatible if the pointer target types are compatible.
* Function types that specify argument types are compatible if the return types are compatible and the argument types are compatible, argument by argument. In addition, they must all agree in whether they use ... to allow additional arguments.
* function types that don’t specify argument types are compatible if the return types are.
* Function types that specify the argument types are compatible with function types that omit them, if the return types are compatible and the specified argument types are unaltered by the argument promotions.


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
* they must agree in their type qualifiers.
* Thus, const int and int are incompatible. It follows that const int * and int * are incompatible too (they are pointers to types that are not compatible).

* If two types are compatible ignoring the qualifiers, we call them nearly compatible.
* Comparison of pointers is valid if the pointers’ target types are nearly compatible.
* If two types are compatible ignoring the qualifiers, and the first type has all the qualifiers of the second type, we say the first is upward compatible with the second.
* Assignment of pointers requires the assigned pointer’s target type to be upward compatible with the right operand (the new value)’s target type.



* In addition, some operations on pointers require operands to have compatible target types.

```c
int value = 10;
int* p1 = &value;
int* p2;

p2 = p1; // VALID: 'int*' is compatible with 'int*'

float pi = 3.14f;
float* pf = &pi;
int* pi;

// ILLEGAL: Compiler will stop this.
pi = pf; // ERROR: 'int*' is not compatible with 'float*'


int scores[] = { 90, 80, 100 };
int* p1 = &scores[0]; // Points to 90
int* p2 = &scores[2]; // Points to 100

if (p1 < p2) { // VALID: 'int*' is compatible with 'int*'
    // This is true
}

int i;
float f;
int* pi = &i;
float* pf = &f;

// ILLEGAL:
if (pi == pf) { ... } // ERROR: Comparing incompatible 'int*' and 'float*'

int scores[] = { 90, 80, 100 };
int* p1 = &scores[0]; // Points to 90
int* p2 = &scores[2]; // Points to 100

// Result is 2 (i.e., 2 'int' elements)
ptrdiff_t diff = p2 - p1;

int i;
float f;
int* pi = &i;
float* pf = &f;

// ILLEGAL:
ptrdiff_t diff = pi - pf; // ERROR: Incompatible pointer types
```
* The Big Exception: __void*__ (The Generic Pointer)

    * The void* type is the one exception to these rules. It is a "generic" pointer that can hold the address of any data type.
    * A void* is compatible for assignment and comparison with all other data pointer types (int*, float*, char*, etc.). This is why malloc returns a void* (it can be assigned to any pointer type) and free accepts a void* (it can free any pointer).
```cpp
int x = 10;
int* pi = &x;

// 1. Assignment
void* pv = pi;  // ✅ Valid: 'int*' can be assigned to 'void*'
pi = pv;      // ✅ Valid: 'void*' can be assigned back to 'int*'

// 2. Comparison
if (pi == pv) { // ✅ Valid: Any pointer can be compared to 'void*'
    // ...
}
```
* The Limitation: You cannot perform pointer arithmetic (like pv + 1 or pv - pv) on a void*. The compiler doesn't know the target type's size, so it has no sizeof(...) to use.

## const
The $\mathbf{const}$ Keyword: Its use for defining read-only data, preventing accidental modification, and placing data in ROM/Flash (read-only memory).
### How the C Compiler Makes a Variable `const`

---

### 1. Const Is Mainly Enforced at Compile-Time
- Compiler marks the variable as **read-only** in its symbol table.
- Attempts to modify it produce a **compile-time error**.
- No special CPU instructions are generated for `const`.

---

### 2. Memory Placement Depends on Declaration Type

#### Global or Static `const`
```c
const int x = 10;
```
- Usually placed in the **`.rodata`** (read-only data) section.
- Runtime writes may cause segmentation fault or undefined behavior.

#### Local `const` Variable
```c
void f() {
    const int a = 10;
}
```
- Stored on the **stack** (always writable).
- Only compile-time protection; no hardware-level guarantee.

---

### 3. `const` Does Not Generate Special Instructions
- No “const opcode”.
- Compiler only enforces rules at compile-time.

---

### 4. `const` Can Be Bypassed (Undefined Behavior)
```c
const int x = 10;
int *p = (int*)&x;
*p = 20;   // undefined behavior
```

---

### 5. Example: Global `const` in Assembly
```asm
.rodata
x:
    .long 10

.text
fun:
    mov eax, DWORD PTR x
    add eax, 1
    ret
```

---

### 6. Local `const` May Be Optimized Away
```c
void f() {
    const int a = 10;
    int y = a + 5;
}
```

Possible optimized assembly:
```asm
mov y, 15
```

---

### Summary Table

| Declaration Type        | Behavior                             | Memory Protected? |
|-------------------------|----------------------------------------|--------------------|
| Local const             | Compile-time rule only                | ❌ No (stack RW)   |
| Global/static const     | Stored in `.rodata`                   | ✔ Yes (if MMU)    |
| Pointer-to-const        | Prevents modification via pointer     | ❌ Can be bypassed |
| Extern const            | Same as global const                  | ✔ If in `.rodata` |
