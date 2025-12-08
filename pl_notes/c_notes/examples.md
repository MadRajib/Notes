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
