### Volatile

- Memory Mapped Example
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