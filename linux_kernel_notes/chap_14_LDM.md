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

### `struct kobject`
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

### `kset` structure
*  purpose of `struct kset` is mainly to group related kernel objects together.
    ```c
    struct kset {
        struct list_head list;
        spinlock_t list_lock;
        struct kobject kobj;
    };
    ```
* Each registered `kset` corresponds to a `sysfs` directory that's created on behalf of its `kobj` element.
* A `kset` can be created and added using the `kset_create_and_add()` function and removed with `kset_unregister()`.
    ```c
    struct kset * kset_create_and_add(const char *name,
                                    const struct kset_uevent_ops *u,
                                    struct kobject *parent_kobj);
    void kset_unregister (struct kset * k);
    ```
    * `name`:  is the name of `kset`, which is also used as the name of the directory that will be created for `kset`.
    * `u`: ptr to `struct uevent_ops`, set of `user events` ops that are called whenever a change is made to `kset`. This can be set to `NULL`.
    * `parent_kobj` : parent kobject of `kset`.

* Example:
    ```c
    static struct kobject foo_kobj, bar_kobj;
    [...]
    example_kset = kset_create_and_add("kset_example",
                            NULL, kernel_kobj);

    /* since we have a kset for this kobject,
    * we need to set it before calling into the kobject core.
    */
    foo_kobj.kset = example_kset;
    bar_kobj.kset = example_kset;

    retval = kobject_init_and_add(&foo_kobj, &foo_ktype, NULL, "foo_name");
    retval = kobject_init_and_add(&bar_kobj, &bar_ktype, NULL, "bar_name");
    ```
* It can be released using `kset_unregister()`
    ```c
    kset_unregister(example_kset);
    ```
### Non-default attributes
* Attributes are sysfs files that are exported to the user space via kobjects.
* An attribute can be readable, writable, or both, from the user space.
    ```c
    struct attribute {
        char *name;
        struct module *owner;
        umode_t mode;
    };
    ```
    * `name` : name of attr : name of file entry.
    * `owner`: attr owner; most of the time this is `THIS_MODULE`
    * `mode`: read/write permissions for this attrb.
* kobject core provides a mechanism where each attrb is embedded in an enclosing and special struct `struct kobj_attribute`; exposes wrapper routines for reading and writing.
    ```c
    struct kobj_attribute {
        struct attribute attr;

        ssize_t (*show)(struct kobject *kobj,
                        struct kobj_attribute *attr, char *buf);
        ssize_t (*store)(struct kobject *kobj,
                        struct kobj_attribute *attr,
                        const char *buf, size_t count);
    };
    ```
* Example:
    ```c
    static ssize_t kobj_attr_show(struct kobject *kobj,
                            struct attribute *attr, char *buf)
    {
        struct kobj_attribute *kattr;
        ssize_t ret = -EIO;
        
        kattr = container_of(attr, struct kobj_attribute, attr);
        if (kattr->show)
            ret = kattr->show(kobj, kattr, buf);
        return ret;
    }

    static ssize_t kobj_attr_store(struct kobject *kobj,
                            struct attribute *attr,
                            const char *buf, size_t count)
    {
        struct kobj_attribute *kattr;
        ssize_t ret = -EIO;
        
        kattr = container_of(attr, struct kobj_attribute, attr);
        if (kattr->store)
            ret = kattr->store(kobj, kattr, buf, count);
        
        return ret;
    }

    const struct sysfs_ops kobj_sysfs_ops = {
        .show = kobj_attr_show,
        .store = kobj_attr_store,
    };
    ```

* To declare attribute statically :
    ```c
    #define __ATTR(_name, _mode, _show, _store) { \
        .attr = {.name = __stringify(_name), \
        .mode = VERIFY_OCTAL_PERMISSIONS(_mode) },\
        .show = _show, \
        .store = _store, \
    }
    ```
    * Example:
    ```c
    static struct kobj_attribute foo_attr =
        __ATTR(foo, 0660, attr_show, attr_store);
    
    static struct kobj_attribute bar_attr =
        __ATTR(bar, 0660, attr_show, attr_store);
    ```
* Now to create the underlying file (add/remove) `sysfs_create_file()` and `sysfs_remove_file()`.
    ```c
    int sysfs_create_file(struct kobject * kobj,
                    const struct attribute * attr);
    void sysfs_remove_file(struct kobject * kobj,
                    const struct attribute * attr);
    ```
* Example:
    ```c
    struct kobject *demo_kobj;
    int err;
    demo_kobj = kobject_create_and_add("demo", kernel_kobj);
    if (!demo_kobj) {
        pr_err("demo: demo_kobj registration failed.\n");
        return -ENOMEM;
    }

    err = sysfs_create_file(demo_kobj, &foo_attr.attr);
    if (err)
        pr_err("unable to create foo attribute\n");

    err = sysfs_create_file(demo_kobj, &bar_attr.attr);
    if (err) {
        sysfs_remove_file(demo_kobj, &foo_attr.attr);
        pr_err("unable to create bar attribute\n");
    }
    ```
    *  the `bar` and `foo` files will be visible in sysfs, in the `/sys/demo` directory.

* Kernel also provides macros for various modes:
    1. `__ATTR_RO(name)` : `0444`
    1. `__ATTR_WO(name)` : `0200`
    1. `__ATTR_RW(name)` : `0644`
    1. `__ATTR_NULL` : List terminator
    
    * These attributes here are built under the assumption that
    the show and store methods are named  `<attribute_name>_show` and
    `<attribute_name>_store`, respectively.
    * Example: 
    ```c
    static struct kobj_attribute attr_foo = __ATTR_RW(foo);
    // this declaration assumes that the show and store methods
    // are defined as foo_show and foo_store, respectively.
    ```
* If want to use same store/show function for all the attributes then attrs should be defined using `__ATTR`.
    * Example:
    ```c
    static ssize_t attr_store(struct kobject *kobj,
                        struct kobj_attribute *attr,
                        const char *buf, size_t count)
    {
        int value, ret;
        ret = kstrtoint(buf, 10, &value);
        
        if (ret < 0)
            return ret;
        if (strcmp(attr->attr.name, "foo") == 0)
            foo = value;
        else /* if (strcmp(attr->attr.name, "bar") == 0) */
            bar = value;
        return count;
    }
    static ssize_t attr_show(struct kobject *kobj,
                            struct kobj_attribute *attr,
                            char *buf)
    {
        int value;
        if (strcmp(attr->attr.name, "foo") == 0)
            value = foo;
        else
            value = bar;
        return sprintf(buf, "%d\n", value);
    }

    // Initialize individual kobj_attributes with shared handlers
    static struct kobj_attribute foo_attr = __ATTR(foo, 0664, attr_show, attr_store);
    static struct kobj_attribute bar_attr = __ATTR(bar, 0664, attr_show, attr_store);
    ```

    * Rather than using strcmp we can use Ids
    ```c
    enum {
    ATTR_FOO_ID = 1,
    ATTR_BAR_ID,
    };

    struct my_attribute {
        struct kobj_attribute kobj_attr;
        int id; // Numeric ID instead of string name
    };

    // Helper macro to get the container struct from the kobj_attribute pointer
    #define to_my_attr(x) container_of(x, struct my_attribute, kobj_attr)

    static int foo = 0;
    static int bar = 0;

    static ssize_t attr_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
    {
        struct my_attribute *m_attr = to_my_attr(attr);
        int value = 0;

        switch (m_attr->id) {
        case ATTR_FOO_ID:
            value = foo;
            break;
        case ATTR_BAR_ID:
            value = bar;
            break;
        default:
            return -EINVAL;
        }

        return sysfs_emit(buf, "%d\n", value); // Safe alternative to sprintf
    }

    static ssize_t attr_store(struct kobject *kobj, struct kobj_attribute *attr,
                            const char *buf, size_t count)
    {
        struct my_attribute *m_attr = to_my_attr(attr);
        int value, ret;

        ret = kstrtoint(buf, 10, &value);
        if (ret < 0)
            return ret;

        switch (m_attr->id) {
        case ATTR_FOO_ID:
            foo = value;
            break;
        case ATTR_BAR_ID:
            bar = value;
            break;
        default:
            return -EINVAL;
        }

        return count;
    }
    ```
### Binary Attr
* There could be situations, although rare, which would require larger data to be exchanged in binary format, for example, all with random access.
* An example of such a situation is a device firmware transfer, where the user space would upload some binary data to be pushed to the hardware or PCI devices, exposing
part or all of their configuration address spaces.
* Note that these attributes are for sending/receiving binary data that is not
interpreted/manipulated by the kernel at all.
    * The only manipulations you can perform are some checks on the magic number and size, for example.
* `struct bin_attribute`:

    ```c
    struct bin_attribute {
        struct attribute attr;
        size_t size;
        void *private;

        ssize_t (*read)(struct file *filp,
                        struct kobject *kobj,
                        struct bin_attribute *attr,
                        char *buffer, loff_t off, size_t count);
        ssize_t (*write)(struct file *filp,
                        struct kobject *kobj,
                        struct bin_attribute *attr,
                        const char *buffer,
                        loff_t off, size_t count);
        int (*mmap)(struct file *filp, struct kobject *kobj,
                        struct bin_attribute *attr,
                        struct vm_area_struct *vma);
    };
    ```
    * `size` represents the maximum size of the binary attribute,
    * `private` : used for any convenience: assigned to buff of the binary attr.
    * `read`, `write` and `mmap`: funcs are optional; works similar to char driver equivalent.
        * `flip` : opened file ptr instance associted with attr.
        * `kobj`: underlying `kobject` associated with the binary attr.
        * `buffer`: op/ip buffer for read/write ops.
        * `off`: file offset.
        * `count`: number for bytes to read/write. 
* Note
    * larger data are always requested/sent on a `PAGE_SIZE` chunk basis.
    * that means `write` can be called multiple times for a single load.
    * This split is handled by kernel, which means it's transparent for the driver.
    * Disadvantage: sysfs has no way of signaling the end of a series of write operations, so code implementation of binary attr has to handle it.

* Two ways to allocate: statically and dynamically:
    ```c
    #define __BIN_ATTR(_name, _mode, _read, _write, _size) { \
        .attr = { .name = __stringify(_name), .mode = _mode }, \
        .read = _read, \
        .write = _write, \
        .size = _size, \

    // It works similarly to the __ATTR macro
    ```
* Like classic attributes, binary attributes have their own high-level helper
macros:

    ```c
    BIN_ATTR_RO(name, size)
    BIN_ATTR_WO(name, size)
    BIN_ATTR_RW(name, size)
    ```
    * These macros declare a single instance of `struct bin_attribute`, whose corresponding variable is named `bin_attribute_<name>`
    
    ```c
    #define BIN_ATTR_RW(_name, _size) \
        struct bin_attribute bin_attr_##_name = \
                    __BIN_ATTR_RW(_name, _size)
    ```
    * like classic attributes, these high-level macros expect the read/write methods to be named `<attribute_name>_read` and `<attribute_name>_write`, respectively.
    
    ```c
    #define __BIN_ATTR_RW(_name, _size) \
    __BIN_ATTR(_name, 0644, _name##_read, _name##_write, \
            _size)
    ```
* For dynamic allocation, a simple `kzalloc()` is enough.
    * However, dynamically allocated binary attributes must be initialized using `sysfs_bin_attr_init()`:
    
    ```c
    void sysfs_bin_attr_init(strict bin_attribute *bin_attr);
    ```

    * After this, the driver must set other properties, such as the underlying
    attribute's mode, name, and permission, and optionally the read/write/map
    functions.
    * Unlike classic attributes, which can be set up as default attributes, binary attributes must be created explicitly using `sys_create_bin_file()`
    ```c
    int sysfs_create_bin_file(struct kobject *kobj, struct bin_attribute *attr);
    ```
    * To remove:
    ```c
    int sysfs_remove_bin_file(struct kobject *kobj, struct bin_attribute *attr);
    ```

    * Example : `drivers/i2c/i2cslave-eeprom.c`
    ```c
    struct eeprom_data {
    [...]
        struct bin_attribute bin;
        u8 buffer[];
    };

    static int i2c_slave_eeprom_probe(
                                struct i2c_client *client)
    {
        struct eeprom_data *eeprom;
        int ret;
        unsigned int size = FIELD_GET(I2C_SLAVE_BYTELEN, id->driver_data) + 1;
        
        eeprom = devm_kzalloc(&client->dev,
                        sizeof(struct eeprom_data) + size,
                        GFP_KERNEL);
        if (!eeprom)
            return -ENOMEM;
        [...]

        sysfs_bin_attr_init(&eeprom->bin);
        eeprom->bin.attr.name = "slave-eeprom";
        eeprom->bin.attr.mode = S_IRUSR | S_IWUSR;
        eeprom->bin.read = i2c_slave_eeprom_bin_read;
        eeprom->bin.write = i2c_slave_eeprom_bin_write;
        eeprom->bin.size = size;
        
        ret = sysfs_create_bin_file(&client->dev.kobj, &eeprom->bin);
        
        if (ret)
            return ret;
        [...]
        
        return 0;
    };

    static int i2c_slave_eeprom_remove(struct i2c_client *client)
    {
        struct eeprom_data *eeprom = i2c_get_clientdata(client);
        
        sysfs_remove_bin_file(&client->dev.kobj, &eeprom->bin);
        [...]
        
        return 0;
    }

    static ssize_t i2c_slave_eeprom_bin_read(struct file *filp,
                        struct kobject *kobj, struct bin_attribute *attr,
                        char *buf, loff_t off, size_t count)
    {
        struct eeprom_data *eeprom;
        eeprom = dev_get_drvdata(kobj_to_dev(kobj));
        [...]
            memcpy(buf, &eeprom->buffer[off], count);
        [...]
        return count;
    }

    static ssize_t i2c_slave_eeprom_bin_write(
                    struct file *filp, struct kobject *kobj,
                    struct bin_attribute *attr,
                    char *buf, loff_t off, size_t count)
    {
        struct eeprom_data *eeprom;
        eeprom = dev_get_drvdata(kobj_to_dev(kobj));
        [...]
            memcpy(&eeprom->buffer[off], buf, count);
        [...]
        return count;
    }

    ```

