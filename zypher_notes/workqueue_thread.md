# Workqueue Threads

| Symbol    | Meeing   |
|--------   |--------  |
|WQ         |WorkQueue |
|WI         |Work Items|

* `k obj` uses a dedicated thread to process work items in FIFO manner.
* each `WI` is associated with a `callback` func.
* used by an `ISR/High-priority` thread to offload non-urgent processing to a lower priority thread, so ti doesn't impact time sensitive processing.
* Any no.of `WQ` can be defined referenced by its mem address.

    ### Key Prop:
    * A `queue` of `WI` that have been added but not yet processed.
    * A `thread` that processes the `WI` in the queue.
        * Priority of thread is configurable allowing it to be cooperative or preemption as required.

* Regardless of priority `WQ thread` will yield between each submitted `WI`, to prevent a cooperative `WQ` from starving other threads.
* `WQ` must be Initiated before it can be used. This sets its queue to empty & spawns the `WQ` thread.
    * thread run forever but sleeps when no work items are available.

### WI Lifecycle
* Any no.of `WI` cna be denied. Each work item is referenced by its mem addr.
* The handler func accepts a single argument, which is the address of the `WI` itself.
* `WI` also maintains info about its status.
* During init of `WI` its status is changed to `not pending`.
* A `WI` may be `queued (K_WORK_QUEUED)` by submitting into a `WQ` by an ISR or a thread.
* Submitting a `WI` appends the `WI` to the `WQ`.
* A `delayable` `WI` item may be `scheduled (K_WORK_DELAYED)` to a `WQ`.

    __K_WORK_RUNNING__      -> running on `WQ`.\
    __K_WORK_CANCELING__    -> if it stated running before a thread has requested that it be cancelled.

    ### Diff Stages of `WI`
    * running on a queue.
    * marked cancelling -> coz a thread used `k_work_cancel_sync()` to wait until the work item completed.
    * queued to run again on the same queue.
    * scheduled to be submitted to a queue.
* `k_work_is_pending`:  return true if the `WI` has outstanding activity such as, queued, delayed/scheduled, running or cancelling.
* `k_work_busy_get()`: returns a bitmask of busy states such as 
    `K_WORK_RUNNING`, `K_WORK_CANCELLING`, `_QUEUED`, `_DELAYED`.
* Re submitting already queued `WI` is ignored, handler is only executed once.
* A handler func is permitted to re-submit the `WI` argument to `WQ` since the `WI` is no longer queued at that time.

### Delayable Work
* When required to delay a work by an `ISR`, scheduling a delayed `WI` is to be submitted to a `WQ`.
* A delayable `WI`, has additional fields that record when & where the item should be submitted.
* When scheduled req is made kernel initiates a timeout mechanism that is triggered after the specified delay has elapsed.
* Once the timeout occurs the kernel submits the `WI` to the specified `WQ`, where it remains queued until it is processed in the standard manner.
* Work handler still gets a pointer to the underlying no delayable work structure which is not publicly accessible from `k_work_delayable`. To access.
```c
static void work_handler(struct k_work *work)
{
    struct k_work_delayable *dwork = k_work_delayable_from_work(work);
    struct work_context *ctx = CONTAINER_OF(dwork, struct work_context, timed_work);
    ...
}
```

### System WorkQueue
The kernel defines a `WQ` known as the `system workqueue` which is available to any application or kernel code that required workqueue support.

### How to use Workqueues
* A `WQ` is defined a variable of type `k_work_q`. 
* it is initialed by:
    * defining a stack area used by its thread.
    * initializing the `k_work`, either zeroing its memory or calling `k_work_queue_init()` and then calling `k_work_queue_start()`.
    * the stack area must be properly defined using `K_THREAD_STACK_DEFINE` to ensure it is properly set up in memory.
    ```c
    #define MY_STACK_SIZE 512
    #define MY_PRIORITY 5

    K_THREAD_STACK_DEFINE(my_stack_area, MY_STACK_SIZE);
    
    struct k_work_q my_work_q;
    
    k_work_queue_init(&my_work_q);
    k_work_queue_start(&my_work_q, my_stack_area,
                    K_THREAD_STACK_SIZEOF(my_stack_area), MY_PRIORITY, NULL);
    ```
    * Queue identity and certain thread rescheduling can be controlled by the optional final params.

* To Interact with `WQ`:  
    * `k_work_queue_drain()`:
        * can be used to `block` the caller until the `WQ` has no items left.
        * `WI` resubmitted from the `WQ` thread are accepted while a queue is draining, but the `WI` from any other thread or ISR are rejected.
        * `plug` flag keeps the submission-rejection in effect even after the drain finishes (`k_work_queue_drain(&queue, true)`). 
        * It is useful if the thread that called drain wants a window to exclusive control.
        * it has to explicitly unplug afterwards(`k_work_queue_unplug`) to resume normal submissions.
        * Delayable `WI` have their own timer running independently of the `WQ` item list.
        * draining only concerns itself with items already in the queue- it doesn't touch or cancel pending timers for delayable items.
        * So: if a delayable items's timer is running and its deadline expires while the queue is `plugged`, the timer fires and tried to submit the item to the queue- but since the queue is plugged that submission is silently rejected. No error is raised, no callback fires, the `WI` just doesn't get queued.

        eg. 
        ```c
        #include <zephyr/kernel.h>
        #include <zephyr/sys/printk.h>

        K_THREAD_STACK_DEFINE(my_wq_stack, 1024);
        static struct k_work_q my_wq;

        /* A normal (immediate) work item */
        static void normal_handler(struct k_work *work)
        {
            printk("normal work executed\n");
        }

        /* A delayable work item — has its own timer */
        static void delayable_handler(struct k_work *work)
        {
            /* If this prints, the item made it into the queue */
            printk("delayable work executed\n");
        }

        static K_WORK_DEFINE(normal_work, normal_handler);
        static K_WORK_DELAYABLE_DEFINE(delayed_work, delayable_handler);

        void demo(void)
        {
            k_work_queue_init(&my_wq);
            k_work_queue_start(&my_wq, my_wq_stack,
                                K_THREAD_STACK_SIZEOF(my_wq_stack),
                                K_PRIO_PREEMPT(5), NULL);

            /* Submit a normal item and a delayable item that fires in 100ms */
            k_work_submit_to_queue(&my_wq, &normal_work);
            k_work_schedule_for_queue(&my_wq, &delayed_work, K_MSEC(100));

            /* Drain AND plug: block here until the queue is empty,
            * then keep rejecting new submissions from other threads/ISRs
            * until we explicitly unplug.
            */
            int ret = k_work_queue_drain(&my_wq, true /* plug */);
            printk("drain returned %d, queue is now plugged\n", ret);

            /* --- Danger window ---
            * The delayed_work timer is STILL running independently.
            * If we sleep past its 100ms deadline while plugged...
            */
            k_sleep(K_MSEC(200));

            /* ...the timer already expired and tried to submit delayed_work
            * to my_wq. Because the queue was plugged, that submission was
            * silently dropped. "delayable work executed" never printed,
            * and there's no error code to tell you it happened.
            */

            /* Some other thread trying to submit here would also be rejected: */
            /* k_work_submit_to_queue(&my_wq, &some_other_work); // rejected, no error */

            /* Now allow submissions again */
            k_work_queue_unplug(&my_wq);
            printk("queue unplugged\n");
        }
        ```
    * `k_work_queue_unplug()` : removes any previous block on submission to the queue due to previous drain operation.

### Submitting a `WI`
* A `WI` is defined using a variable of type `k_work`.
* It is initialized by calling `k_work_init()` or during compile time using `K_WORK_DEFINE`.
* Initiated `WI` can be submitted to the `system WQ` by calling `k_work_submit()` or to a specified `WQ` by calling `k_work_submit_to_queue()`. 

    eg :
    ```c
    struct device_info {
        struct k_work work;
        char name[16]
    } my_device;

    void my_isr(void *arg)
    {
        ...
        if (error detected) {
            k_work_submit(&my_device.work);
        }
        ...
    }

    void print_error(struct k_work *item)
    {
        struct device_info *the_device =
            CONTAINER_OF(item, struct device_info, work);
        printk("Got error on device %s\n", the_device->name);
    }

    /* initialize name info for a device */
    strcpy(my_device.name, "FOO_dev");

    /* initialize work item for printing device's error messages */
    k_work_init(&my_device.work, print_error);

    /* install my_isr() as interrupt handler for the device (not shown) */
    ...
    ```
* Following APIs can be used to check the status of or synchronize with the `WI`:
    * `k_work_busy_get()` returns a snapshot of flags indicating work item state.
        * a zero value indicated work is not scheduled, submitted, being executed or otherwise being referenced by the `WQ` infrastructure.
    * `k_work_is_pending()` returns `true` if and only if the work is scheduled, queued or running.
    * `k_work_flush()` invoked from thread and used to block until the `WI` has completed. It returns immediately if the work is not pending. 
    * `k_work_cancel()` attempts to prevent the work item from being executed. May or may not be successful. ISRs safe.
    * `k_work_cancel_sync()` may be invoked from threads to block until the work completes. it will return immediately if the cancellation was successful or not necessary. This can be called after `k_work_cancel()` is invoked to confirm completion of an ISR-initiated cancellation.

### Scheduling a delayable `WI`
* Defined using a var type `k_work_delayable`.
* to initialize `k_work_init_delayable()` or `K_WORK_DELAYABLE_DEFINE`
* There are two APIs that submit work after a delay:
    * `k_work_schedule()` or `k_work_schedule_for_queue()`schedules work to be executed at a specific time or after a delay.
        * run 100 ms after the first event and ignore the later events until then.
        * If timer is already running ignore it.
        * USE_case: UART receiving char & you want to process the while burst 100ns after it started regardless of how many keeps coming in.
    * `k_work_reschedule()` or `k_work_reschedule_for_queue()` unconditionally sets the deadline for the work, replacing any previous incomplete delay and changing the destination queue if necessary. 
        * Always reset the timer.
        * Snooze restart the timer.
* If the work item is not scheduled both APIs behave the same.
* If `K_NO_WAIT` is specified as the delay the behavior is as if the item was immediately submitted directly to the target queue, without waiting for a minimal timeout.
* The helper function `k_work_delayable_from_work()` can be used to get a pointer to the containing `k_work_delayable` from a pointer to `k_work` that is passed to a work handler function.
* The following additional API can be used to check the status of or synchronize with the work item:
    * `k_work_delayable_busy_get()` is the analog to `k_work_busy_get()` for delayable work.
    * `k_work_delayable_is_pending()` is the analog to `k_work_is_pending()` for delayable work.
    * `k_work_flush_delayable()` is the analog to `k_work_flush()` for delayable work.
    * `k_work_cancel_delayable()` is the analog to `k_work_cancel()` for delayable work; similarly with `k_work_cancel_delayable_sync()`.
### `WQ` Best Practices
* __AVOID Race Conditions__
    * Shared state touched by both a work handler and other threads/ISRs needs synchronization — atomic ops for simple flags, `k_spinlock/k_sem/k_mutex` for more complex cases.
    * __Don't block the work thread on a sleeping lock__ — it starves other queued items. Take locks with `K_NO_WAIT` inside a handler; if it fails, resubmit the work instead of waiting.
        ```c
        static void work_handler(struct work *work)
        {
                struct work_context *parent = CONTAINER_OF(work, struct work_context,
                                                        work_item);

                if (k_mutex_lock(&parent->lock, K_NO_WAIT) != 0) {
                        /* NB: Submit will fail if the work item is being cancelled. */
                        (void)k_work_submit(work);
                        return;
                }

                /* do stuff under lock */
                k_mutex_unlock(&parent->lock);
                /* do stuff without lock */
        }
        ```
    * __Danger:__ if a lower-priority thread holds the lock, a tight resubmit loop can starve it out (priority inversion–like effect). Prefer a __delayable work item rescheduled with a non-zero delay__ in this situation, not an immediate resubmit.
        ```c
        /* Workqueue thread runs at high priority.
        * lower_prio_thread (holds parent->lock) runs at low priority.
        */
        static void work_handler(struct k_work *work)
        {
            struct ctx *parent = CONTAINER_OF(work, struct ctx, work_item);

            if (k_mutex_lock(&parent->lock, K_NO_WAIT) != 0) {
                /* BAD: immediately resubmits, so this handler runs again
                * almost instantly. On a high-priority workqueue thread,
                * this can preempt/starve lower_prio_thread before it
                * ever gets CPU time to release the lock.
                */
                (void)k_work_submit(work);
                return;
            }
            /* ... */
            k_mutex_unlock(&parent->lock);
        }
        ```
    Good Approach
        ```c
        static K_WORK_DELAYABLE_DEFINE(my_work, work_handler);

        static void work_handler(struct k_work *work)
        {
            struct k_work_delayable *dwork = k_work_delayable_from_work(work);
            struct ctx *parent = CONTAINER_OF(dwork, struct ctx, work_item);

            if (k_mutex_lock(&parent->lock, K_NO_WAIT) != 0) {
                /* GOOD: back off for 10ms, giving lower_prio_thread
                * a real chance to run and release the lock.
                */
                (void)k_work_reschedule(dwork, K_MSEC(10));
                return;
            }
            /* ... do stuff under lock ... */
            k_mutex_unlock(&parent->lock);
        }
        ```
    * Resubmitting from inside a handler can fail if the item is being cancelled — usually fine (cancellation will complete once handler returns), but if you need to know, add explicit signaling.
        ```c
        static K_WORK_DELAYABLE_DEFINE(poll_work, poll_handler);
        static atomic_t poll_cancelled = ATOMIC_INIT(0);

        static void poll_handler(struct k_work *work)
        {
            struct k_work_delayable *dwork = k_work_delayable_from_work(work);

            /* do polling work ... */

            int ret = k_work_reschedule(dwork, K_MSEC(50));
            if (ret < 0) {
                /* Usually fine to ignore: if cancellation is in flight,
                * the pending k_work_cancel_delayable() will still complete.
                * But if the caller needs to *know* the reschedule was
                * dropped due to cancellation, an atomic flag makes that
                * explicit instead of guessing from the return code alone.
                */
                if (atomic_get(&poll_cancelled)) {
                    printk("poll stopped: cancellation confirmed\n");
                }
            }
        }

        void stop_polling(void)
        {
            atomic_set(&poll_cancelled, 1);   /* explicit signal, set first */
            (void)k_work_cancel_delayable(&poll_work);
        }
        ```
    * Work items are self-locking for submit/schedule purposes — no external lock needed just to call submit/schedule. But if resubmission depends on external state, you need an independent self-locking signal (e.g., atomic cancel flag) to stop a delayable item from re-arming after cancellation.
        ```c
        static K_WORK_DELAYABLE_DEFINE(heartbeat_work, heartbeat_handler);
        static atomic_t heartbeat_active = ATOMIC_INIT(1);

        static void heartbeat_handler(struct k_work *work)
        {
            struct k_work_delayable *dwork = k_work_delayable_from_work(work);

            if (!atomic_get(&heartbeat_active)) {
                /* External flag says "stop" — don't re-arm, even though
                * calling reschedule() itself would be perfectly safe
                * (submit/schedule are self-locking on their own).
                */
                return;
            }

            /* send heartbeat ... */

            /* No external lock needed just to call this — the work item
            * handles its own internal locking for submission.
            */
            (void)k_work_reschedule(dwork, K_SECONDS(1));
        }

        void heartbeat_stop(void)
        {
            /* Set the flag BEFORE cancelling. Without this flag, a race is
            * possible: cancel could run right as the handler is between
            * "finished sending" and "reschedule call", and the item would
            * silently re-arm itself one more time after the cancel call
            * appears to have completed.
            */
            atomic_set(&heartbeat_active, 0);
            (void)k_work_cancel_delayable(&heartbeat_work);
        }
        ```
* __Check Return Values__
    * Every work API call returns a status — check it.
    * `k_work_submit_to_queue()` can fail (item being cancelled, or queue not accepting items) → work silently won't run.
    * `k_work_cancel() / k_work_cancel_delayable()` can return while the handler is still mid-run → touching shared state right after is a race.
    * If deliberately ignoring a return value, cast to `(void)` and document why the failure case is safe.