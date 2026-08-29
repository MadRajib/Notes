# Timers
* A kernel object that uses kernel's system clock.
* When timer's specified time limit is reached it can perform app define action or it can simply record the expiration and wait for app to read it status.
* Any no of timer can be defined, limited by mem.
    ### Key Properties
    * Duration: Time interval before the timer expires for the first time. (`k_timeout_t` value).
    * Period: Time interval between all timer expirations after the first one, also a `k_timeout_t`.
        * A period of `K_NO_WAIT` or `K_FOREVER` is a one-shot timer that stops after a single expiration.
    * Expiry function: Executed each time the timer expires.
        * The func is executed by the system clock interrupt handler.
    * Stop function: Executed if the timer is stopped prematurely while running. Func is executed by the thread that stops the timer.
    * Status: value that indicates how many times the timer has expired since the status value was last read.
----
* Initializing timer specifies its expiry function and stop function values, sets the timer's status to zero, and puts the timer into the `stopped` state.
* Started by specifying a duration and a period.
    * Timer's status is reset to zero and then the timer enters the running state and begins counting down towards expiry.
----
* Status value of a running timer is incremented when it expires and expiry func is called.
* threads waiting on that timer are unblocked.
* If the timer's period is zero the timer enters the stopped state.
* Otherwise the timer restarts with a the countdown equal to the delta between the time the timer "should have expired" and its period.
    * eg. say period was 100ms, so idealy expiry should happen at 100ms, 200ms, 300 ms. 
    * But due to some blocking factors (interrupted disabled, high priority isr), first expiry happened at 105ms.
    * Now the system is trigger the next expiry on 200ms and not at 205ms.
    * Hence there is no drift.
----
* A running timer can be restarted in mid-countdown.
* Timer's status is reset to zero then the timer begins counting down
using the new duration and period values specified by the caller.
* If a thread is waiting on the timer, it continues waiting.

----
* Reading a timer's status resets its value to zero.
* The amount of time remaining before the timer expires can also be read; a value of zero indicates that the timer is stopped.

-----
* A thread may read a timer's status indirectly by `synchronizing` with the timer.
* this blocks the thread until the timer's status is non-zero (indicating that it has expired at least once) or the timer is stopped.
* The synchronization operation return the timer's status and resets it to zero.

### Timer Observers
* When `CONFIG_TIMER_OBSERVER` is enabled, code can register timer observers that are notified of timer lifecycle events across all timers in the system.
* An observer is a set of optional callbacks invoked when a timer is initialized, started, stopped, or expires.
* An observer is defined statically with `K_TIMER_OBSERVER_DEFINE`, which takes pointers to the `on_init`, `on_start`, `on_stop`, and `on_expiry` callbacks.
* Any callback that is not needed may be passed as NULL.
* Because the expiry callback runs in interrupt context, it must be kept short and non-blocking.

### Implementation

__Defining a Timer__
* A timer is defined using a variable of type `k_timer`.
* It must then be initialized by calling `k_timer_init()` or statically using `K_TIMER_DEFINE`
    ```c
    struct k_timer my_timer;
    extern void my_expiry_function(struct k_timer *timer_id);

    k_timer_init(&my_timer, my_expiry_function, NULL);


    //Statically
    K_TIMER_DEFINE(my_timer, my_expiry_function, NULL);
    ```
__USing a Timer Expiry Function__
* Below code uses a timer to perform a non-trivial action on a periodic basic.
* Since the work cannot be done at the interrupt level, the timer's expiry function submits a work item to system workqueue.
    ```c
    void my_work_handler(struct k_work *work)
    {
        /* do the processing that needs to be done periodically */
        ...
    }

    K_WORK_DEFINE(my_work, my_work_handler);

    void my_timer_handler(struct k_timer *dummy)
    {
        k_work_submit(&my_work);
    }

    K_TIMER_DEFINE(my_timer, my_timer_handler, NULL);

    ...

    /* start a periodic timer that expires once every second */
    k_timer_start(&my_timer, K_SECONDS(1), K_SECONDS(1));
    ```
__Reading Timer Status__

* The following code reads a timer’s status directly to determine if the timer has expired or not.
    ```c
    K_TIMER_DEFINE(my_status_timer, NULL, NULL);

    ...

    /* start a one-shot timer that expires after 200 ms */
    k_timer_start(&my_status_timer, K_MSEC(200), K_NO_WAIT);

    /* do work */
    ...

    /* check timer status */
    if (k_timer_status_get(&my_status_timer) > 0) {
        /* timer has expired */
    } else if (k_timer_remaining_get(&my_status_timer) == 0) {
        /* timer was stopped (by someone else) before expiring */
    } else {
        /* timer is still running */
    }
    ```
__Using Timer Status Synchronization__
* The following code performs timer status synchronization to allow a thread to do useful work while ensuring that a pair of protocol operations are separated by the specified time interval.
    ```c
    K_TIMER_DEFINE(my_sync_timer, NULL, NULL);

    ...

    /* do first protocol operation */
    ...

    /* start a one-shot timer that expires after 500 ms */
    k_timer_start(&my_sync_timer, K_MSEC(500), K_NO_WAIT);

    /* do other work */
    ...

    /* ensure timer has expired (waiting for expiry, if necessary) */
    k_timer_status_sync(&my_sync_timer);

    /* do second protocol operation */
    ...
    ```
### Suggested Uses
* Use a timer to initiate an asynchronous operation after a specified amount of time.
* Check if N time has elapsed (more precise/flexible than `k_sleep()/k_usleep()`).
* Do other work while enforcing a time-bounded operation.
* For measuring how long an operation itself took, read the clock directly — don't use a timer for that.