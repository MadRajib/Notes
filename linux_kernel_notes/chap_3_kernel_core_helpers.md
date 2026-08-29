
**Shared resources** : accessible by several contenders.

Two synchronization mechanisms:
1. `Locks`: Used for mutual exclusion. When one contender holds the lock, no other can hold it. Eg spinlocks and mutexes.
1. `Conditional Variables`: For waiting for a change.

> Linux kernel does not implement conditional variables.

Conditional variable functionality is achieve kernel provides:
1. `Wait queue` : to wait for a change
1. `Completion queue` : To wait for the completion of a given computation, mostly used with DMAs.

These mechanism is exposed to drivers as set of `Application programming interfaces (APIs)`.

### Spinlocks
- hardware-based locking primitive, depends on hw capabilities for `atomic` operations.
- Spinning only happens on multi-core machines
    - Coz in single core system the task either holds the spinlock and proceeds or never runs until the lock is released.
- A spinlock is said to be lock held by CPU, in contrast to a mutex which is a lock held by a task.
- It operates by disabling the scheduler on the local CPU.
    - That means the task currently running on that CPU cannot be preempted except by `interrupt requests (IRQs)` if they are not disabled on the local CPU.
    - This makes them suitable for `symmetrical multiprocessing (SMP)` safety and for executing atomic tasks.

**Spinlock usage**
- 