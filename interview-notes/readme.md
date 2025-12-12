# Interview Notes

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