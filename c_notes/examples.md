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