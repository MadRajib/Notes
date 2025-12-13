### Volatile
The $\mathbf{volatile}$ Keyword: to tell the compiler that a variable's value can be changed by external factors (like an Interrupt Service Routine (ISR) or a memory-mapped hardware register) and prevent aggressive compiler optimizations (e.g., caching the value in a register)
- Example Question: "When must you use $\text{volatile}$?"
- (Answer: For shared variables between an ISR and main code, and for memory-mapped I/O registers).

Memory Mapped Example
```c
#define UART_STATUS_REG  (*(volatile uint32_t*)0x40001000)

void wait_for_tx_ready() {
    while ((UART_STATUS_REG & 0x01) == 0) {
        // wait until hardware updates the register
    }
}
```
- ISR Updating a Flag
```c
volatile int data_ready = 0;

void ADC_IRQHandler() {
    data_ready = 1;   // interrupt changes the variable
}

int main() {
    while (!data_ready) {
        // wait for interrupt
    }
}
```
- Multithreading Polling Flag (Not Thread-Safe, Only Polling)
```c
volatile int stop = 0;

void* worker(void* arg) {
    while (!stop) {
        // do work
    }
    return NULL;
}

int main() {
    sleep(1);
    stop = 1;   // signal thread to exit
}
```
- DMA Hardware Updating a Status Variable
```c
volatile uint32_t dma_complete = 0;

void start_dma() {
    DMA->CTRL = 1;

    while (!dma_complete) {
        // wait until DMA hardware writes to memory
    }
}
```
- Prevent Compiler Removing Delay Loop
```c
void delay(volatile int count) {
    while (count--) {
        // loop won't be optimized away
    }
}
```
- Volatile Pointer Example
```c
volatile int *flag = (volatile int*)0x20000010;  // some hardware-mapped RAM

void check_flag() {
    while (*flag == 0) {
        // must reread from memory each time
    }
}
```
### Offsetof implementation
```c
#define offsetof(type, member) \
        ((size_t)((char *)&((type *)0)->member - (char *)0))
```
- Here we are not dereferencing type rather just finding the address of the member.
- Usage:
```c
struct Point {
    int x;      // offset 0
    float y;    // offset 4
};

int main() {
    printf("offset of x = %zu\n", offsetof(struct Point, x));
    printf("offset of y = %zu\n", offsetof(struct Point, y));
    return 0;
}

//Output
offset of x = 0
offset of y = 4
```

### Containerof implementation
```c
#define containerof(ptr, type, member) ({ \
        const typeof(((type *)0)->member) *__mptr = (ptr); \
        (type *)((char *)__mptr - offsetof(type, member));})
```
- Usage:
```c
struct Node {
    int data;
};

struct Wrapper {
    int id;
    struct Node node;
    float value;
};

int main() {
    struct Wrapper w = { .id = 10, .node = { 99 }, .value = 3.5 };

    // ptr points to the embedded Node
    struct Node *node_ptr = &w.node;

    // recover the parent (Wrapper) pointer
    struct Wrapper *wrap_ptr = container_of(node_ptr, struct Wrapper, node);

    printf("Recovered id = %d\n", wrap_ptr->id);      // prints 10
    printf("Recovered value = %.1f\n", wrap_ptr->value); // prints 3.5
    return 0;
}
// output
Recovered id = 10
Recovered value = 3.5
```
### Union Struct trick
Example 1:
```c
    union V32 {
        struct {
            I32 x;
            I32 y;
        };

        I32 v[2];
    };

    // Usage
    union V32 vec;

    // Write using struct fields
    vec.x = 10;
    vec.y = 20;

    printf("vec.x = %d\n", vec.x);
    printf("vec.y = %d\n", vec.y);

    // Access same data using array view
    printf("vec.v[0] = %d\n", vec.v[0]);
    printf("vec.v[1] = %d\n", vec.v[1]);

    // Modify using array
    vec.v[0] = 100;
    vec.v[1] = 200;

    // Access again via struct
    printf("vec.x = %d\n", vec.x);
    printf("vec.y = %d\n", vec.y);
```
Example 2:
```c
#include <stdint.h>

typedef union {
    uint32_t REG;   // full 32-bit access

    struct {
        uint32_t ENABLE : 1;   // bit 0
        uint32_t MODE   : 1;   // bit 1
        uint32_t COUNT  : 6;   // bits 2–7
        uint32_t RESERVED : 24;
    } B;                       // bitfield view

} CTRL_REG_t;

#define CTRL_REG   (*(volatile CTRL_REG_t*)0x40001000)

// Access the full register
uint32_t r = CTRL_REG.REG;

// Modify individual bitfields
// directly writing to hardware reg
CTRL_REG.B.ENABLE = 1;
CTRL_REG.B.MODE = 0;
CTRL_REG.B.COUNT = 15;

// Modify full register
CTRL_REG.REG = 0x000000FF;
```

### Ring buffer

#### With Locks:
```c

typedef struct {
    void **buf;
    size_t capacity; // number of slots
    size_t head;     // read index
    size_t tail;     // write index
    pthread_mutex_t mtx;
    pthread_cond_t not_empty;
    pthread_cond_t not_full;
} ring_t;


void ring_init(size_t capacity) {
    // intialize buffer
    r->buf = calloc(capacity, sizeof(void*));
    if (!r->buf) { free(r); return NULL; }

    r->capacity = capacity;
    r->head = r->tail = 0;

    pthread_mutex_init(&r->mtx, NULL);
    pthread_cond_init(&r->not_empty, NULL);
    pthread_cond_init(&r->not_full, NULL);
}

bool ring_push(ring_t *r, void *item) {
    pthread_mutex_lock(&r->mtx);

    while (ring_full(r))
        pthread_cond_wait(&r->not_full, &r->mtx);

    r->buf[r->tail] = item;
    r->tail = (r->tail + 1) % r->capacity;

    pthread_cond_signal(&r->not_empty);
    pthread_mutex_unlock(&r->mtx);
    return true;
}

bool ring_pop(ring_t *r, void **out) {
    pthread_mutex_lock(&r->mtx);

    while (ring_empty(r))
        pthread_cond_wait(&r->not_empty, &r->mtx);

    *out = r->buf[r->head];
    r->head = (r->head + 1) % r->capacity;

    pthread_cond_signal(&r->not_full);
    pthread_mutex_unlock(&r->mtx);
    return true;
}

// Producer thread
void* producer_thread(void *arg) {
    ring_t *r = arg;

    for (int i = 1; i <= 30; i++) {
        printf("Producer: pushing %d\n", i);
        ring_push(r, i);
        usleep(100 * 1000); // Sleep 100ms
    }

    return NULL;
}

// Consumer thread
void* consumer_thread(void *arg) {
    ring_t *r = arg;

    for (int i = 0; i < 30; i++) {
        int val = ring_pop(r);
        printf("Consumer: popped %d\n", val);
        usleep(150 * 1000); // Sleep 150ms
    }

    return NULL;
}

int main(.....) {
    ...
    pthread_t prod, cons;

    pthread_create(&prod, NULL, producer_thread, &r);
    pthread_create(&cons, NULL, consumer_thread, &r);

    pthread_join(prod, NULL);
    pthread_join(cons, NULL);
}

```

### Producer Comsumer uisng semaphores
```c

#define N 8
int buffer[N];
int head = 0;
int tail = 0;

sem_t empty;  // available slots
sem_t full;   // available items


void* producer(void* arg) {
    for (int i = 0; i < 30; i++) {
        sem_wait(&empty);              // wait for space

        buffer[tail] = i;
        tail = (tail + 1) % N;

        sem_post(&full);               // item available
    }
    return NULL;
}

void* consumer(void* arg) {
    for (int i = 0; i < 30; i++) {
        sem_wait(&full);               // wait for item

        int val = buffer[head];
        head = (head + 1) % N;

        sem_post(&empty);              // slot freed
    }
    return NULL;
}
```

### SPSC using lock free ring buffer
- Only one thread writes tail
- Only one thread writes head
- Both threads may read the other index
- No mutex, no condition variable
- Correctness relies on atomic operations + memory ordering
- Core Idea:
    - Producer owns tails
    - Consumer owns head
    - Each side only reads other index.

```c
#include <stdatomic.h>
#include <stdbool.h>
#include <stddef.h>

typedef struct {
    size_t capacity;
    void **buffer;

    atomic_size_t head;  // consumer-owned
    atomic_size_t tail;  // producer-owned
} spsc_ring_t;

// Intialisation
bool spsc_init(spsc_ring_t *r, size_t capacity) {
    r->buffer = calloc(capacity, sizeof(void *));
    if (!r->buffer)
        return false;

    r->capacity = capacity;
    atomic_init(&r->head, 0);
    atomic_init(&r->tail, 0);
    return true;
}

// Producer
bool spsc_push(spsc_ring_t *r, void *item) {
    // Only gurantees atomic no ordering or synchronisation
    // CPU and compiler are free to reorder surrounging instructions
    size_t tail = atomic_load_explicit(&r->tail, memory_order_relaxed);
    size_t next = (tail + 1) % r->capacity;
    // all the mem operations performed after the atomic load will occur
    // after the data with atomic variable has be loaded.
    // this guranteees that all the changes made by predceding release operations
    // are visbible
    size_t head = atomic_load_explicit(&r->head, memory_order_acquire);

    if (next == head) {
        // buffer full
        return false;
    }

    r->buffer[tail] = item;
    // ensures that all the data read/write performed before realse
    // become visible before the atomic operation itself
    atomic_store_explicit(&r->tail, next, memory_order_release);
    return true;
}

// Consumer
bool spsc_pop(spsc_ring_t *r, void **out) {
    size_t head = atomic_load_explicit(&r->head, memory_order_relaxed);

    size_t tail = atomic_load_explicit(&r->tail, memory_order_acquire);

    if (head == tail) {
        // buffer empty
        return false;
    }

    *out = r->buffer[head];

    size_t next = (head + 1) % r->capacity;
    atomic_store_explicit(&r->head, next, memory_order_release);
    return true;
}

```
- Why memory ordering is required here ?
    - To ensure: Data must be visible before tail is visible.
    - atomic_store_explicit(&tail, next, memory_order_release);
        - All writes before this store
        - Are visible to any thread that does an acquire load of tail
    - atomic_load_explicit(&tail, memory_order_acquire);
        - All writes before the release store
        - Are visible after this load

cpp varient
```cpp
void enqueue(const Item& item) {
    size_t current_head = head.load(std::memory_order_relaxed);
    size_t next_head = (current_head + 1) & (N - 1);

    // 1. Check for fullness
    while (next_head == tail.load(std::memory_order_acquire)) {
        // Queue is full, spin-wait or yield
    }

    // 2. Write the data
    buffer[current_head] = item;
    
    // 3. Update the head (Release semantics guarantees the write to 'buffer' is visible)
    head.store(next_head, std::memory_order_release);
}

bool dequeue(Item& item) {
    size_t current_head = head.load(std::memory_order_acquire);
    size_t current_tail = tail.load(std::memory_order_relaxed);
    
    // 1. Check for emptiness
    if (current_tail == current_head) {
        return false; // Queue is empty
    }

    // 2. Read the data (Acquire semantics guarantees the read is ordered after 'head' load)
    item = buffer[current_tail];
    
    // 3. Update the tail
    size_t next_tail = (current_tail + 1) & (N - 1);
    tail.store(next_tail, std::memory_order_release);

    return true;
}
```

### Print memory layout of a process
```c
/* This file is part of the sample code and exercises
 * used by the class "Advanced Programming in the UNIX
 * Environment" taught by Jan Schaumann
 * <jschauma@netmeister.org> at Stevens Institute of
 * Technology.
 *
 * This file is in the public domain.
 *
 * You don't have to, but if you feel like
 * acknowledging where you got this code, you may
 * reference me by name, email address, or point
 * people to the course website:
 * https://stevens.netmeister.org/631/
 */

/* This program allows us to visualize the layout of a
 * process in memory, printing the addresses of
 * various parts of the program, including components
 * in the text, data, and bss segments as well as on
 * the heap and the stack.
 *
 * This program can also illustrate a stack overflow
 * if compiled with '-DSTACKOVERFLOW'.
 */

#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define ARRAY_SIZE 16
#define MALLOC_SIZE 32

char array[ARRAY_SIZE];
char *string = "a string";
char *string2;
int num = 10;
int num2;

extern char **environ;

void func(int);
void func2(const char *);

int
main(int argc, char **argv) {
	int var;
	char *ptr;

	char func_array[ARRAY_SIZE];

	(void)printf("High address (args and env):\n");
	(void)printf("----------------------------\n");
	(void)printf("environ[0] at                                          : 0x%12lX\n", (unsigned long)environ);
	(void)printf("last arg at                                            : 0x%12lX\n", (unsigned long)&argv[argc]);
	(void)printf("first arg at                                           : 0x%12lX\n", (unsigned long)&argv[0]);
	(void)printf("\n");

	(void)printf("Stack:\n");
	(void)printf("------\n");
	(void)printf("First variable inside main at                          : 0x%12lX\n", (unsigned long)&var);
	(void)printf("func_array[] ends at                                   : 0x%12lX\n", (unsigned long)&func_array[ARRAY_SIZE]);
	(void)printf("func_array[] (like 'array[]', but on stack) begins at  : 0x%12lX\n", (unsigned long)&func_array[0]);

	(void)printf("argc at                                                : 0x%12lX\n", (unsigned long)&argc);
	(void)printf("argv at                                                : 0x%12lX\n", (unsigned long)&argv);

	func2("from main");
	func(0);

	(void)printf("\n");

	(void)printf("Heap:\n");
	(void)printf("-----\n");
	if ((ptr = malloc(MALLOC_SIZE)) == NULL) {
		err(EXIT_FAILURE, "unable to allocate memory");
		/* NOTREACHED */
	}

	(void)printf("malloced area ends at                                  : 0x%12lX\n", (unsigned long)ptr+MALLOC_SIZE);
	(void)printf("malloced area begins at                                : 0x%12lX\n", (unsigned long)ptr);
	free(ptr);
	(void)printf("\n");

	(void)printf("Uninitialized Data (BSS):\n");
	(void)printf("-------------------------\n");
	(void)printf("array[] ends at                                        : 0x%12lX\n", (unsigned long)&array[ARRAY_SIZE]);
	(void)printf("array[] (uninitialized, fixed-size char * on BSS) from : 0x%12lX\n", (unsigned long)&array[0]);
	(void)printf("num2 (uninitialized global int) at                     : 0x%12lX\n", (unsigned long)&num2);
	(void)printf("string2 (uninitialized global char *) at               : 0x%12lX\n", (unsigned long)&string2);
	(void)printf("extern **environ at                                    : 0x%12lX\n", (unsigned long)&environ);
	(void)printf("\n");

	(void)printf("Initialized Data:\n");
	(void)printf("-----------------\n");
	(void)printf("num (initialized global int) at                        : 0x%12lX\n", (unsigned long)&num);
	(void)printf("string (initialized global char *) at                  : 0x%12lX\n", (unsigned long)&string);
	(void)printf("\n");

	(void)printf("Text Segment:\n");
	(void)printf("-------------\n");
	(void)printf("func2 (function) at                                    : 0x%12lX\n", (unsigned long)&func2);
	(void)printf("func (function) at                                     : 0x%12lX\n", (unsigned long)&func);
	(void)printf("main (function) at                                     : 0x%12lX\n", (unsigned long)&main);
	(void)printf("\n");

	return EXIT_SUCCESS;
}


void
func(int recurse) {
	int fint;
	char *msg = "from func";

	/* Change this value to 0 and note how
	 * the location of where it is stored
	 * changes from the Data to BSS segment. */
	static int n = 1;
	(void)printf("func frame at                                          : 0x%12lX\n", (unsigned long)&fint);

	if (recurse) {
		msg = "recursive";
	}
	(void)printf("static int n within func at                            : 0x%12lX\n", (unsigned long)&n);
	printf("func (called %5d times): frame at                    : 0x%12lX\n", n, (unsigned long)&fint);

	n++;
	func2(msg);
}

void
func2(const char *how) {
	int fint;
	(void)printf("func2 (%s): frame at                            : 0x%12lX\n", how, (unsigned long)&fint);
#ifdef STACKOVERFLOW
	func(1);
#endif
}
```

```bash
High address (args and env):
----------------------------
environ[0] at                                          : 0x7FFE40BD1F08
last arg at                                            : 0x7FFE40BD1F00
first arg at                                           : 0x7FFE40BD1EF8

Stack:
------
First variable inside main at                          : 0x7FFE40BD1DA4
func_array[] ends at                                   : 0x7FFE40BD1DC0
func_array[] (like 'array[]', but on stack) begins at  : 0x7FFE40BD1DB0
argc at                                                : 0x7FFE40BD1D9C
argv at                                                : 0x7FFE40BD1D90
func2 (from main): frame at                            : 0x7FFE40BD1D74
func frame at                                          : 0x7FFE40BD1D6C
static int n within func at                            : 0x5D0DFEE01014
func (called     1 times): frame at                    : 0x7FFE40BD1D6C
func2 (from func): frame at                            : 0x7FFE40BD1D34

Heap:
-----
malloced area ends at                                  : 0x5D0E09B096D0
malloced area begins at                                : 0x5D0E09B096B0

Uninitialized Data (BSS):
-------------------------
array[] ends at                                        : 0x5D0DFEE01040
array[] (uninitialized, fixed-size char * on BSS) from : 0x5D0DFEE01030
num2 (uninitialized global int) at                     : 0x5D0DFEE01048
string2 (uninitialized global char *) at               : 0x5D0DFEE01040
extern **environ at                                    : 0x5D0DFEE01020

Initialized Data:
-----------------
num (initialized global int) at                        : 0x5D0DFEE01010
string (initialized global char *) at                  : 0x5D0DFEE01018

Text Segment:
-------------
func2 (function) at                                    : 0x5D0DFEDFE6A0
func (function) at                                     : 0x5D0DFEDFE5DB
main (function) at                                     : 0x5D0DFEDFE209
```