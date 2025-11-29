# Lesser known tricks, quirks and features of C
> source https://jorenar.com/blog/less-known-c

## \#\# Variables declaration in parentheses
Names of variables can be wrapped in parentheses in declarations:
```c
int (v);
void (*p);
float (ar1)[16];
double (ar1[8]);
int x, (*y), (z);
```

## \#\# Array pointers
Decay-to-pointer makes regular pointers to array usually not needed:
```c
int arr[10];

int *ap0 = arr;        // array decay-to-pointer; the array "decays to a pointer to its firt element"
// ap0[2] = ...

int (*ap1)[10] = &arr; // proper pointer to array; *ap1 is the full array
// (*ap1)[2] = ...
```
But ability to allocate a big multi-dimensional array on heap is nice:
```c
int (*ap3)[90000][90000] = malloc(sizeof *ap3);
```
* Here, ap3 is a pointer to a 2D array of fixed size: 90000 × 90000.
* sizeof *ap3 automatically calculates the size of the whole array (i.e., 90000 × 90000 × sizeof(int)).

* What Happens with int **arr (Pointer-to-Pointer)?
```c
int **arr = malloc(n * sizeof(int *));    // array of pointers
for (int i = 0; i < n; ++i)
    arr[i] = malloc(m * sizeof(int));    // each row separately allocated
```
* Each arr[i] points to a separately allocated block of m ints.
* The entire 2D array is NOT guaranteed to be contiguous in memory.
* Cache inefficiency due to fragmented allocations.
* Pointer arithmetic gets complicated.
* Compiler has no knowledge of dimensions at the type level — arr is just int**, so nothing enforces array size.


* Once allocated, you can access elements as:
```c
(*ap3)[i][j] = value;
```
* You avoid complex pointer-of-pointer juggling (int **arr) which doesn’t guarantee contiguous memory.
* This approach ensures one contiguous block of memory.
* You get correct type safety, as the compiler knows the exact dimensions.

With pointers even VLA can find its use:
```c
int n = 1000;
int (*ap4)[n] = malloc(sizeof *ap4);
```
* ap4 is now a pointer to an array of size n.
* sizeof *ap4 becomes n × sizeof(int).
* Unlike int *arr = malloc(n * sizeof(int)); where you lose explicit type relation between ap4 and the array size, this keeps the type tightly coupled.
* The compiler can catch type mismatches.