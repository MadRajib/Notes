### LDM
* `LDM` relies on 3 lowest level DS:
    1. `kobject`
    1. `kobj_type`
    1. `kset`

* `sysfs`: in-mem virtual file system
    * shows hierarchy of kernel objs
    * abstracted by instances of `struct kobject`.
* `attribute`: `sysfs attribute` : appears as a file in sysfs.
    * It can be mapped to anything: variable, device prop, buffer or
    * anything useful that may need to be exported to the world.
* For each directory found under `sysfs` there is a `struct kobject` wandering around in the kernel.
* A `kobject` can export one or more attrs, which appear in that Kobject's sysfs directory as a file.

```c
struct kobject {
    const char          *name;      // name of kobject
    struct list_head    entry;
    struct kobject      *parent;    // parent kobject
    struct kset         *kset;
    struct kobj_type    *ktype;     // describes kobj
    struct sysfs_dirent *sd;
    struct kref         kref;       // for reference counting, 1 when initialize
    [...]
}
```
* `name` field can be modified using `kobject_set_name(struct kobject *kobj, const char *name)`. Used as a name of this `kobject` directory.
* `sd` : points to `struct ssfs_dirent` -> directory of this kobj in sysfs.
    * `name` will be used as the name of this dir.
    * `parent` is set, then this dir will be sub-dir in the parent's dir.
* `ktype`: Each kobj is given a set of default attrs when its created.
    * allows kobj to share common ops (`sysfs_ops`).
* `kset` : which set (group) of objs this obj belongs to.

*  To allocate and Initialize :
    * `kzalloc()` (or `kmalloc()`) or `kobject_create()`
    * if `kzalloc()` -> `kobject_init()` must be called to initialize
    * `kobject_create()` : allocate + initialize

    ```c
    void kobject_init(struct kobject *kobj, struct kobj_type *ktype) // ktype can't be NULL

    struct kobject *kobject_create(void)
    ```
* `kobject_add()` from driver to link this obj with the system.
    * if `parent` is not set it can be set when adding.
    ```c
    int kobject_add(struct kobject *kobj, struct kobject *parent, const char *fmt, ...)
    ```
* `kobject_init_and_add()` -> init + add (needs to be allocated first!)
    ```c
    int kobject_init_and_add(   struct kobject *kobj,
                                struct kobj_type    *ktype,
                                struct kobject  *parent,
                                struct char *fmt, ...);
    ```
* `kobject_create_and_add()` -> alloc + init + add
    ```c
    struct kobject * kobject_create_and_add(const char *name, struct kobject *parent);
    ```

* Predefine kobjects:
    * `kernel_kobj` : responsible for `/sys/kernel` dir.
    * `mm_kobj`:    responsible for `/sys/kernel/mm`.
    * `fs_kobj`: filesystem kobject and it's responsible for `/sys/fs`.
    * `hypervisor_kobj` : `/sys/hypervisor`.
    * `power_kobj`: power management kobject, for `/sys/power`
    * `firmware_kobj`: for `/sys/firmware` dir.
* Once done with kobject the driver should release it.
    * `kobject_release()` : doesn't consider other users.
    * `kobject_put()` is recommended which decrements ref count and releases if reaches 0 by calling `kobject_release()`.
* Recommended to wrap kobject's usage into `kobject_get()` and `kobject_put()`.
    ```c
    void kobject_put(struct kobject * kobj);
    struct kobject *kobject_get(struct kobject *kobj);
    ```c
    static struct kobject *mykobj;
    [...]
    mykobj = kobject_create();
    if (!mykobj)
        return -ENOMEM;

    kobject_init(mykobj, &my_ktype);
    if (kobject_add(mykobj, NULL, "%s", "hello")) {
        pr_info("ldm: kobject_add() failed\n");
        
        kobject_put(mykobj);
        mykobj = NULL;
        return -1;
    }
    ```
    ```c
    static struct kobject * class_kobj = NULL;
    static struct kobject * devices_kobj = NULL;
    
    /* Create /sys/class */
    class_kobj = kobject_create_and_add("class", NULL);
    if (!class_kobj)
        return -ENOMEM;
    [...]
    
    /* Create /sys/devices */
    devices_kobj = kobject_create_and_add("devices", NULL);
    if (!devices_kobj)
        return -ENOMEM;

    ```

### `kobj_type` structure
* defines the behavior of a kobject element and controls what happens to this kobject when it is created or destroyed.
* Also contains the default attributes of the kobject
* as well as the hooks that allow it to operate on these attributes.
* Every kobject must have an associated `kobj_type` structure
    ```c
    struct kobj_type {
        void (*release)(struct kobject *);
        const struct sysfs_ops sysfs_ops;
        struct attribute **default_attrs;
    };
    ```
    * `release` is a callback that's called on the release path of the kobject to give drivers a chance to release resources.
    * This callback is implicitly run when `kobject_put()` is about to free the kobject.
    * `default_attrs` is an array of pointers to attribute structures. This field lists the attributes to be created for every kobject of this type.
    * while `sysfs_ops` provides a set of methods that allow you to access those attributes.
    ```c
    struct sysfs_ops {
        ssize_t (*show)(struct kobject *kobj,
                        struct attribute *attr, char *buf);
        ssize_t (*store)(struct kobject *kobj,
                        struct attribute *attr, const char *buf,
                        size_t size);
    };
    ```
    * `show` is the callback that's invoked in response to a read operation of an attribute being exposed by this `kobj_type` – that is, whenever an attribute is read from the user space.
        * `buf` is the output buffer (`PAGE_SIZE` in length).
            * The data that must be exposed must be put inside buf, preferably using `scnprintf()`.
            * Finally, if the callback succeeds, it must return the size (in bytes) of the data that was written into the buffer, or a negative error if it fails
    * `store` is called for writing purposes – that is, when users write something into an attribute.
        * Its `buf` parameter is PAGE_SIZE at most but it can be smaller.
        * It must return the size (in bytes) of the data that was read from the buffer on success or a negative error on failure.