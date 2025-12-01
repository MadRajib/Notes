* Fundamental Ideas behind classes are __data abstraction__ and __encapsulation__.
* __Data Abstraction__ -> technique that relies on the seperation of __interface__ and __implementation__.
    > The __interface__ of a class consists
    of the operations that users of the class can execute.

    > The __implementation__ includes
    the class’ data members, the bodies of the functions that constitute the interface,
    and any functions needed to define the class that are not intended for general use.

* __Encapsulation__ enforces the separation of a class' interface and implementations i.e users of the class can use the interface but have no access to the implemtation.
    > A class that is encapsulated hides its implementation
    
* A class that uses data abstraction and encapsulation defines an __abstract data type__.
    > In an abstract data type, the class designer worries about how the class is implemented.

    > Programmers who use the class need not know how the type works.They can instead think __*abstractly*__ about what the type does.

* Use the __input (>>)__ and __output (<<)__ operators to __read__ and __write__ objects. 
    > src .eg cin >> dst and dst .eg cout << src
* Difference between __class__ and __struct__
  > In absence of an access-specifier for a base class, public is assumed when the derived class is declared struct and private is assumed when the class is declared class.
* __endl__ which is a special value called manipulator and has the effect of
ending the current line and flushing the buffer associated with that device.
* Difference between __member__ and __non-member__ function.
    > member functions are function inclued inside the class, needed directly for class functioning. Must be declared inside the class.
    
    > Non member class belongs to same header but dosen't needed for actual
    functioning of the class. Declared outside the class.
* __<<__ operator 
    ```cpp
    std::cout << "Enter two numbers:" << std::endl;
    (std::cout << "Enter two numbers:") << std::endl;
    std::cout << "Enter two numbers:";
    std::cout << std::endl;
* __>>__ op[erator
    ```cpp
    std::cin >> v1 >> v2;
    (std::cin >> v1) >> v2;
    std::cin >> v1;
    ```
* Reading an Unknown Number of Inputs
    ```cpp
    // read until end-of-file, calculating a running total of all values read
        while (std::cin >> value)
            sum += value; // equivalent to sum = sum + value
    ```
* Use an __unsigned__ type when you know that the values cannot be negative.

* Use __int__ for integer arithmetic. short is usually too small and, in practice, long often has the same size as int. If your data values are larger than the minimum guaranteed size of an int, then use __long long__.

* Do not use plain __char__ or __bool__ in arithmetic expressions. Use them only to hold characters or truth values. Computations using char are especially problematic because char is signed on some machines and unsigned on others. If you need a tiny integer, explicitly specify either signed char or unsigned char.

* Use __double__ for __floating-point computations__; float usually does not have enough precision, and the cost of double-precision calculations versus single-precision is negligible. In fact, on some machines, double-precision operations are faster than single. The precision offered by long double usually is unnecessary and often entails considerable run-time cost.
* __List initialization__ issue . The compiler will not let us list initialize variables of built-in type if the initializer might lead to the loss of information:
    ```cpp
    long double ld = 3.1415926536;
    int a{ld}, b = {ld}; // error: narrowing conversion required
    int c(ld), d = ld;   // ok: but value will be truncated
    ```
* Uninitialized objects of built-in type defined inside a function body have undefined value. Objects of class type that we do not explicitly initialize have a value that is defined by the class.
* Once initialized, a __reference__ remains bound to its initial object. There is no way to rebind a __reference__ to refer to a different object. Because there is no way to rebind a __reference__, __references__ must be initialized.
* Because __references__ are not objects, we may not define a __reference__ to a __reference__.
* A __reference__ may be bound only to an object, not to a literal or to the result of a more general expression:
    ```cpp
    int &refVal4 = 10;   // error: initializer must be an object
    double dval = 3.14;
    int &refVal5 = dval; // error: initializer must be an int object
    ```
*  Unlike a reference, a __pointer__ is an object in its own right. __Pointers__ can be assigned and copied; a single pointer can point to several different objects over its lifetime.
* Unlike a reference, a __pointer__ need not be initialized at the time it is defined.
* Because __references__ are not objects, they don’t have addresses.
* Define const variable for other files to use
    ```cpp
    // file_1.cc defines and initializes a const that is accessible to other files
    extern const int bufSize = fcn();
    // file_1.h
    extern const int bufSize; // same bufSize as defined in file_1.cc
    ```
* We can bind a __reference__ to an object of a __const__ type. A reference to const cannot be used to change the object to which the reference is bound.
    ```cpp
    const int ci = 1024;
    const int &r1 = ci;   // ok: both reference and underlying object are const
    r1 = 42;              // error: r1 is a reference to const
    int &r2 = ci;         // error: non const reference to a const object
    ```
* Technically speaking, there are no const references. A reference is not an object, so we cannot make a reference itself const. Indeed, because there is no way to make a reference refer to a different object, in some sense all references are const.
* we can bind a reference to const to a nonconst object, a literal, or a more general expression.
    ```cpp
    int i = 42;
    const int &r1 = i;      // we can bind a const int& to a plain int object
    const int &r2 = 42;     // ok: r1 is a reference to const
    const int &r3 = r1 * 2; // ok: r3 is a reference to const
    int &r4 = r * 2;        // error: r4 is a plain, non const reference
    ```
    ```cpp
    const int temp = dval;   // create a temporary const int from the double
    const int &ri = temp;    // bind ri to that temporary

    int &rj = temp; // error binding to non const obj
    ```
* Because the underlying object might be nonconst, it might be changed by other means
    ```cpp
        int i = 42;
    int &r1 = i;          // r1 bound to i
    const int &r2 = i;    // r2 also bound to i; but cannot be used to change i
    r1 = 0;               // r1 is not const; i is now 0
    r2 = 0;               // error: r2 is a reference to const
    ```
* Plane pointer cannot store address of const types:
    ```cpp
    const double pi = 3.14;   // pi is const; its value may not be changed
    double *ptr = &pi;        // error: ptr is a plain pointer
    const double *cptr = &pi; // ok: cptr may point to a double that is const
    *cptr = 42;               // error: cannot assign to *cptr
    ```
* __const__ pointer
    ```cpp
    int errNumb = 0;
    int *const curErr = &errNumb;  // curErr will always point to errNumb
    const double pi = 3.14159;
    const double *const pip = &pi; // pip is a const pointer to a const object
    ```
* The fact that a pointer is itself const says nothing about whether we can use the pointer to change the underlying object. Whether we can change that object depends entirely on the type to which the pointer points.
    ```cpp
    *pip = 2.72;     // error: pip is a pointer to const
    // if the object to which curErr points (i.e., errNumb) is nonzero
    if (*curErr) {
        errorHandler();
        *curErr = 0; // ok: reset the value of the object to which curErr is bound
    }
    ```
* We use the term __top-level const__ to indicate that the pointer itself is a const. When a pointer can point to a const object, we refer to that const as a __low-level const__.
    ```cpp
    int i = 0;
    int *const p1 = &i;  // we can't change the value of p1; const is top-level
    const int ci = 42;   // we cannot change ci; const is top-level
    const int *p2 = &ci; // we can change p2; const is low-level
    const int *const p3 = p2; // right-most const is top-level, left-most is not
    const int &r = ci;  // const in reference types is always low-level
    ```
* When we copy an object, top-level consts are ignored:
    ```cpp
    i = ci;  // ok: copying the value of ci; top-level const in ci is ignored
    p2 = p3; // ok: pointed-to type matches; top-level const in p3 is ignored
    ```
* On the other hand, low-level const is never ignored. When we copy an object, both objects must have the same low-level const qualification or there must be a conversion between the types of the two objects. In general, we can convert a nonconst to const but not the other way round:
    ```cpp
    int *p = p3; // error: p3 has a low-level const but p doesn't
    p2 = p3;     // ok: p2 has the same low-level const qualification as p3
    p2 = &i;     // ok: we can convert int* to const int*
    int &r = ci; // error: can't bind an ordinary int& to a const int object
    const int &r2 = i; // ok: can bind const int& to plain int
    ```
* __constexpr__ : A constant expression is an expression whose value cannot change and that can be evaluated at compile time. A literal is a constant expression. A const object that is initialized from a constant expression is also a constant expression.
* Whether a given object (or expression) is a constant expression depends on the types and the initializers.
    ```cpp
    const int max_files = 20;    // max_files is a constant expression
    const int limit = max_files + 1; // limit is a constant expression
    int staff_size = 27;       // staff_size is not a constant expression, coz its not const int
    const int sz = get_size(); // sz is not a constant expression, coz runtime calculated
    ```
* Under the new standard, we can ask the compiler to verify that a variable is a constant expression by declaring the variable in a constexpr declaration. Variables declared as constexpr are implicitly const and must be initialized by constant expressions:
    ```cpp
    constexpr int mf = 20;        // 20 is a constant expression
    constexpr int limit = mf + 1; // mf + 1 is a constant expression
    constexpr int sz = size();    // ok only if size is a constexpr function
    ```
* The types we can use in a constexpr are known as __“literal types”__ because they are simple enough to have literal values.
* we can define both pointers and reference as constexprs.
* when constptr is applied to pointers, the constexpr specifier applies to the pointer, not the type to which the pointer points:
    ```cpp
    const int *p = nullptr;     // p is a pointer to a const int
    constexpr int *q = nullptr; // q is a const pointer to int
    ```
* A constexpr pointer may point to a const or a nonconst type:
    ```cpp
    constexpr int *np = nullptr; // np is a constant pointer to int that is null
    int j = 0;
    constexpr int i = 42;  // type of i is const int
    // i and j must be defined outside any function
    constexpr const int *p = &i; // p is a constant pointer to the const int i
    constexpr int *p1 = &j;      // p1 is a constant pointer to the int j
    ```
* Variables defined inside a function ordinarily are not stored at a fixed address. Hence, we cannot use a constexpr pointer to point to such variables. On the other hand, the address of an object defined outside of any function is a constant expression, and so may be used to initialize a constexpr pointer.
* Functions may define variables that exist across calls to that function. Like an object defined outside any function, these special local objects also have fixed addresses. Therefore, a constexpr reference may be bound to, and a constexpr pointer may address, such variables.
## Dara sturcture usesage

1.  Iterate through a vector<T>
```cpp
vector<int> nums;
// 1
for (auto x: nums) { // we can place int in place of auto
    // do something with x
}
// 2
for (auto it = begin (nums); it != end (nums); ++it) {
    // do something with *it
}
// also can be written as
for (auto it = nums.begin(); it != v.end(); ++it)

// 3
for (int i =0; i < nums.size() ; i++) {
    //do something with nums[i]
}

```