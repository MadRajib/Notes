# Interview Notes

1. [What happens in between interrupt and interrupt handler ?](#what-happens-in-between-interrupt-and-interrupt-handler-)
1. [Producer Consumer with mutexes Interview question](#producer-consumer-with-mutexes-interview-question)

### What happens in between interrupt and interrupt handler ?
Ans:

- When an interrupt occurs, the flow of execution is temporarily diverted from the currently running code to handle the interrupt.
- Here’s what typically happens between the occurrence of an interrupt and the execution of its interrupt handler:

    - **Interrupt Signal**: A hardware device, timer, or software generates an interrupt signal.
    - **Interrupt Acknowledgment**: The CPU acknowledges the interrupt signal, either immediately (if interrupts are enabled) or after finishing the current instruction.

    - **Saving Context**: The CPU saves the current state of the execution. This usually includes:

        - The program counter (PC), which keeps track of where the CPU was executing.
        The processor status register (e.g., flags, privilege level).
        - Other necessary registers depending on the architecture (general-purpose registers, stack pointer, etc.).
    - **Disabling Further Interrupts**: Often, the CPU will disable further interrupts (masking) to avoid nested interrupts while the current interrupt is being processed.

    - **Determining the Interrupt Source**: The CPU or the interrupt controller identifies which interrupt has occurred (in systems with multiple possible sources). This could be a fixed vector or handled via an interrupt controller (e.g., PIC, APIC) that prioritizes and maps interrupts.

    - **Jump to the Interrupt Vector**: The CPU loads the interrupt vector, which is the address of the corresponding interrupt handler function.

    - **Executing the Interrupt Handler**: The interrupt handler (or ISR, Interrupt Service Routine) is executed. It performs the necessary task to handle the interrupt, such as reading from hardware, processing input, or resetting a timer.

    - **Restoring Context**: Once the interrupt handler finishes, the CPU restores the saved context (the state of the program prior to the interrupt).

    - **Resuming Execution**: Finally, execution resumes from where it was interrupted.

    In embedded systems, especially in user space on Linux, managing these steps involves configuring both hardware (via device drivers) and handling the interrupt in kernel space, which then notifies user space if necessary.

### How does all these work ?
Ans:
- The responsibility for handling these steps is shared between hardware and software. Here's a breakdown of who does what:

    1. **Interrupt Signal (Hardware)**:
    A hardware device (e.g., keyboard, timer, network card) or software generates the interrupt signal.
    2. **Interrupt Acknowledgment (Hardware)**:
    The CPU acknowledges the interrupt. In systems with an interrupt controller (e.g., Programmable Interrupt Controller (PIC) or Advanced Programmable Interrupt Controller (APIC)), the interrupt controller helps prioritize and route the interrupt to the CPU.
    3. **Saving Context (Hardware)**:
    The CPU automatically saves a minimal context (e.g., program counter and status registers). Depending on the CPU architecture, some registers might need to be saved by the interrupt handler software as well.
    4. **Disabling Further Interrupts (Hardware/Software)**:
    The CPU typically disables further interrupts when handling an interrupt. However, the interrupt handler (software) might enable nested interrupts if needed.
    5. **Determining the Interrupt Source (Hardware/Software)**:
    In a simple system, the CPU directly handles this.
    In more complex systems, an interrupt controller (e.g., PIC or APIC) helps determine the interrupt source, and the operating system (software) reads this information to route to the correct handler.
    6. **Jump to the Interrupt Vector (Hardware)**:
    The CPU fetches the interrupt vector from a specific location in memory (often pre-configured during system initialization) and jumps to the address of the appropriate interrupt handler.
    7. **Executing the Interrupt Handler (Software)**:
    The operating system or device driver provides the actual interrupt handler code (software) that is executed in response to the interrupt. This code processes the interrupt (e.g., reading data from a hardware device).
    8. **Restoring Context (Hardware/Software)**:
    The CPU handles the minimal context restoration automatically.
    The interrupt handler (software) might restore additional context, such as general-purpose registers, depending on the system.
    9. **Resuming Execution (Hardware)**:
    The CPU resumes executing the interrupted process once the interrupt handler completes.
    
    In summary, the hardware (CPU, interrupt controllers, devices) handles the low-level signaling, saving of basic context, and interrupt vector fetching. The operating system and device drivers (software) manage the higher-level tasks of processing the interrupt and restoring the full state.

### Producer Consumer with mutexes Interview question
1. Does this code compile and run correctly?

    Lead-level answer:

    No, there are several correctness issues.
    ring_init() does not take a ring_t * parameter but dereferences r. It also returns void while trying to return NULL.
    ring_push() expects a void *, but the producer passes an int.
    ring_pop() returns bool, but the consumer treats it as returning an int.
    These are API contract violations that must be fixed before runtime correctness.

1. How do you distinguish full vs empty in this ring buffer?

    Lead-level answer:

    With only head and tail, head == tail is ambiguous.
    I resolve this by keeping one slot empty, so the buffer is full when (tail + 1) % capacity == head.
    This simplifies logic without requiring an extra variable, at the cost of one slot.

1. Show ring_empty() and ring_full()

    Answer:
    ```c
    bool ring_empty(ring_t *r) {
        return r->head == r->tail;
    }

    bool ring_full(ring_t *r) {
        return ((r->tail + 1) % r->capacity) == r->head;
    }
    ```

    Effective capacity is capacity - 1, which I would document clearly.

1. Why while instead of if for pthread_cond_wait()?

    Lead-level answer:

    Because condition variables can wake up spuriously, and because multiple threads may be awakened by a single signal.
    The condition must always be re-checked under the mutex.
    Using if can cause consumers to read from an empty buffer or producers to overwrite data.

1. Will this work with multiple producers and consumers?

    Lead-level answer:

    Yes, the design is MPMC-safe.
    The mutex serializes access to shared state, and the condition variables are guarded by while loops.
    However, fairness is not guaranteed, and under heavy contention some threads may starve.

1. Why signal after updating head/tail and not before?

    Lead-level answer:

    The condition must be true before signaling.
    If I signal before modifying the state, the awakened thread may immediately go back to sleep, causing a lost wakeup.

1. Can this code deadlock?

    Lead-level answer:

    The algorithm itself does not deadlock if invariants are correct.
    Deadlocks can occur if:

    - ring_full() / ring_empty() logic is wrong

    - A thread exits while holding the mutex

    - Error paths forget to unlock the mutex

1. Who owns the memory passed via void *item?

    Lead-level answer:

    Ownership transfers from producer to consumer.
    The ring buffer only stores pointers; it does not manage memory lifetime.
    This separation avoids hidden allocation costs and keeps the structure generic.

1. Is it safe to push pointers to stack memory?

    Lead-level answer:

    No.
    Stack memory may go out of scope before the consumer accesses it, leading to undefined behavior.

1. How would you safely pass integers?

    Lead-level answer:

    Prefer storing integers by value instead of pointers.
    Alternatively, allocate memory dynamically or pass integers via intptr_t with strict documentation.

1. Time complexity of push/pop?

    Answer:

    Both operations are O(1).

1. Main performance bottleneck?

    Lead-level answer:

    Mutex contention and kernel transitions in pthread_cond_wait().
    Under high load, cache line bouncing also becomes significant.

1. How would you make this lock-free?

    Lead-level answer:

    For SPSC, I’d use atomic head/tail indices with memory ordering guarantees.
    For MPMC, I’d use sequence counters per slot or a well-tested algorithm like Dmitry Vyukov’s bounded queue.
    Lock-free correctness is hard, so I’d only do this when profiling proves mutexes are the bottleneck.

1. Any resource leaks?

    Lead-level answer:

    Yes.
    Mutexes, condition variables, and the buffer must be destroyed explicitly.
    In long-running systems, this would be unacceptable.

1. How do you support graceful shutdown?

    Lead-level answer:

    Add a closed flag protected by the mutex.
    On shutdown, set it and broadcast on both condition variables so all waiters wake up and exit cleanly.

1. How do you add timeout support?

    Lead-level answer:

    Replace pthread_cond_wait() with pthread_cond_timedwait() and return a timeout error to the caller.

1. Can a signal be missed?

    Lead-level answer:

    Yes. Condition variables do not queue signals.
    That’s why the condition must always be checked under the mutex.

1. Why is a mutex mandatory with condition variables?

    Lead-level answer:

    To protect shared state and to avoid race conditions between checking the condition and going to sleep.
    A condition variable without a mutex is meaningless.

1. What if signal is sent when no one is waiting?

    Lead-level answer:

    The signal is lost.
    That’s acceptable because correctness relies on state, not on signals.

1. How would you integrate this into an epoll-based server?

    Lead-level answer:

    I would decouple notification from storage.
    The ring buffer would store data, and an eventfd would be written to on push.
    Consumers would wait on epoll, avoiding condition variables and enabling unified IO and data events.

1. When would you prefer semaphores over this design?

    Lead-level answer:

    When I only need counting and don’t need to protect complex shared state.
    Semaphores are simpler but less expressive than mutex + condition variables.

1. Why not always use lock-free?

    Lead-level answer:

    Lock-free code is harder to reason about, debug, and maintain.
    I default to simple, correct code and only move to lock-free after profiling proves the need.

1. What happens if we unlock the mutex first and then signal the other thread?
    ```c
    pthread_mutex_unlock(&r->mtx);
    pthread_cond_signal(&r->not_empty);
    ```

    Lead-level answer:

    This might cause deadlock eg. producer unlocks, consumer gets the lock and before sleeping after the conditon
    producer signals, which is lows, consumer sleep inside cond_wait: deadlock.
