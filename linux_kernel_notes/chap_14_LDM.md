## Table of Contents

- [LDM](#ldm)
    - [BUS DS](#bus-ds)
- [Deep Dive In LDM](#deep-dive-in-ldm)
- [`struct kobject`](#struct-kobject)
- [`kobj_type` structure](#kobj_type-structure)
- [`kset` structure](#kset-structure)
- [Non-default attributes](#non-default-attributes)
- [Binary Attr](#binary-attr)
- [Attribute group](#attribute-group)
- [Symbolic Links](#symbolic-links)
- [Device-, driver-, bus- and class- related attributes](#device--driver--bus--and-class--related-attributes)
- [Making a sysfs attribute poll- and select- compatible](#making-a-sysfs-attribute-poll--and-select--compatible)

### LDM(Linux Device Model)
* LDM Introduced following features:
    * The concept of classes. They are used to group devices of the same type or that expose the same functionalities (for example, mice and keyboards are both input devices).
    * Communication with the user space through a virtual filesystem, allowing
    you to manage and enumerate devices and the properties they expose from
    user space.
    * Object life cycle management using reference counting.
    * A power management facility, allowing you to handle the order in which devices should shut down.
    * The reusability of the code. Classes and frameworks expose interfaces, behaving like a contract that any driver that registers with them must respect.
    * An `object-oriented (OO)`-like programming style and encapsulation in the kernel.
* LDM introduced device hierarchy.
* It is built on top of a few data structures.
    * The bus: `struct bus_type`
    * The device: `struct device`
    * The device driver: `struct device_driver`
#### Bus DS
* A bus is a channel link between devices and the processor.
* `bus controller`: The hardware entity that manages the bus and exports its protocol to devices.
    * Eg: the USB controller provides USB support, while the I2C controller provides I2C bus support.
* However, the bus controller, being a device on its own, must be registered like any device.
    *  It will be the parent of the devices that need to sit on this bus.
    ```c
    struct bus_type {
        const char *name;
        const char *dev_name;
        struct device *dev_root;
        const struct attribute_group **bus_groups;
        const struct attribute_group **dev_groups;
        const struct attribute_group **drv_groups;
        int (*match)(struct device *dev,
                    struct device_driver *drv);
        int (*probe)(struct device *dev);
        void (*sync_state)(struct device *dev);
        int (*remove)(struct device *dev);
        void (*shutdown)(struct device *dev);
        int (*suspend)(struct device *dev, pm_message_t state);
        int (*resume)(struct device *dev);
                    const struct dev_pm_ops *pm;
        [...]
    };
    ```
    * `match`: cb invoked whenever a new device/driver is added to this bus.
    * `probe`: cb invoked whenever a new device/driver is added to this bus after match has occurred.
        * responsible for allocation specific bus device struct and calling driver's `probe` which is suppose to manage the device.
    * `remove`: when device lease this bus.
* Example
    ```c 
    // packt.h
    #ifndef _PACKT_H
    #define _PACKT_H

    #include <linux/device.h>

    struct packt_device {
        const char *name;       /* match key */
        int id;
        struct device dev;
    };
    #define to_packt_device(d) container_of(d, struct packt_device, dev)

    struct packt_driver {
        int  (*probe)(struct packt_device *pdev);
        void (*remove)(struct packt_device *pdev);
        struct device_driver driver;
        // const struct i2c_device_id *id_table;
    };
    
    #define to_packt_driver(d) container_of(d, struct packt_driver, driver)
    
    int packt_register_driver(struct packt_driver *driver);
    void packt_unregister_driver(struct packt_driver *driver);
    int packt_register_device(struct packt_device *packt);
    void packt_unregister_device(struct packt_device *packt);

    struct packt_device * packt_device_alloc(const char *name, int id);

    #endif
    ```
    * Apart from `bus_type`, the bus driver must define a bus-specific driver struct extending generic `struct device_driver`. here `struct packt_driver`.
    * The bus driver must also allocate a bus-specific device struct for each physical devices on the bus. here `struct packt_device`.
    * It is also responsible to setting up device's bus and parent fields, as well as registering them with the LDM core.
        * These fields must point to the `bus_type` and the `bus_device` struct that are defined in bus driver.
    * Each bus manages two important lists:
        1. list of devices that have been added and sitting on it.
        1. list of drivers that have been registered.
    * Bus driver must provide apis to register/unregister device drivers and devices.
        * these apis wraps generic apis from LDM core:
            * `diver_register()`, `device_register`, `diver_unregister()`, `device_unregister`.
        ```c
        // packt_bus.c
        #include <linux/module.h>
        #include <linux/string.h>
        #include "packt.h"
        /*
        * Let's write and export symbols that people
        * writing drivers for packt devices must use.
        */
        int packt_register_driver(struct packt_driver *driver)
        {
            driver->driver.bus = &packt_bus_type;
            return driver_register(&driver->driver);
        }
        EXPORT_SYMBOL(packt_register_driver);
        
        void packt_unregister_driver(struct packt_driver *driver)
        {
            driver_unregister(&driver->driver);
        }
        EXPORT_SYMBOL(packt_unregister_driver);
        
        int packt_register_device(struct packt_device *packt)
        {
            packt->dev.bus = &packt_bus_type;
            return device_register(&packt->dev);
        }
        EXPORT_SYMBOL(packt_device_register);
        
        void packt_unregister_device(struct packt_device *packt)
        {
            device_unregister(&packt->dev);
        }
        EXPORT_SYMBOL(packt_unregister_device);

        // To initialize packt device
        struct packt_device * packt_device_alloc(const char *name, int id)
        {
            struct packt_device *packt_dev;
            int status;
            
            packt_dev = kzalloc(sizeof(*packt_dev), GFP_KERNEL);
            if (!packt_dev)
                return NULL;
            
            /* devices on the bus are children of the bus device */
            strcpy(packt_dev->name, name);
            packt_dev->dev.id = id;

            dev_dbg(&packt_dev->dev, "device [%s] registered with PACKT bus\n",
                                                        packt_dev->name);
            return packt_dev;
        }
        EXPORT_SYMBOL_GPL(packt_device_alloc);
        ```
        * `packt_device_alloc` function allocates a bus-specific device struct that must be used to register a `PACKT` device with the bus.
        * To define `PACKT` controller
        ```c
        //packt_bus.c
        struct packt_controller {
            char name[48];
            struct device dev; /* the controller device */
            struct list_head list;
            int (*send_msg) (stuct packt_device *pdev,
                            const char *msg, int count);
            int (*recv_msg) (stuct packt_device *pdev,
                            char *dest, int count);
        };

        /* system global list of controllers */
        static LIST_HEAD(packt_controller_list);
        struct packt_controller *packt_alloc_controller(struct device *dev)
        {
            struct packt_controller *ctlr;
            if (!dev)
                return NULL;
            ctlr = kzalloc(sizeof(packt_controller), GFP_KERNEL);
            
            if (!ctlr)
                return NULL;
            
            device_initialize(&ctlr->dev);
            [...]
            return ctlr
        }
        EXPORT_SYMBOL_GPL(packt_alloc_controller);
        
        int packt_register_controller(struct packt_controller *ctlr)
        {
            /* must provide at least on hook */
            if (!ctlr->send_msg && !ctlr->recv_msg){
                pr_err("Registering PACKT controller failure\n");
            }
            
            device_add(&ctlr->dev);
            [...] /* other sanity check */
            list_add_tail(&ctlr->list, &packt_controller_list);
        }
        EXPORT_SYMBOL_GPL(packt_register_controller);
        ```
        * Note that after registering a controller, it will appear under `/sys/devices` in sysfs. Any devices that are added to this bus will appear under `/sys/devices/packt-0/`.
    * Bus Registration
        * bus controller itself is a device, in most cases buses are memory mapped platform devices.
        * We should use `bus_register(struct *bufs_type)` to register bus with the kernel.
        ```c
        // packt.c
        /* Return 1 if this driver can handle this device */
        static int packt_match(struct device *dev, const struct device_driver *drv)
        {
            struct packt_device *pdev = to_packt_device(dev);

            return !strcmp(pdev->name, drv->name);
        }

        /* Fill in env vars for udev; MODALIAS lets udev autoload the driver module */
        static int packt_uevent(const struct device *dev, struct kobj_uevent_env *env)
        {
            const struct packt_device *pdev = to_packt_device(dev);

            return add_uevent_var(env, "MODALIAS=packt:%s", pdev->name);
        }

        static int packt_bus_probe(struct device *dev)
        {
            struct packt_device *pdev = to_packt_device(dev);
            struct packt_driver *pdrv = to_packt_driver(dev->driver);

            return pdrv->probe ? pdrv->probe(pdev) : 0;
        }

        static void packt_bus_remove(struct device *dev)
        {
            struct packt_device *pdev = to_packt_device(dev);
            struct packt_driver *pdrv = to_packt_driver(dev->driver);

            if (pdrv->remove)
                pdrv->remove(pdev);
        }
        /* This is our bus structure */
        struct bus_type packt_bus_type = {
            .name = "packt",
            .match = packt_device_match,
            .probe = packt_device_probe,
            .remove = packt_device_remove,
            <!-- .shutdown = packt_device_shutdown, -->
        };

        static int __init packt_init(void)
        {
            int status;
            status = bus_register(&packt_bus_type);
            if (status < 0)
                goto err0;
            status = class_register(&packt_master_class);
            if (status < 0)
                goto err1;
            return 0;
        err1:
            bus_unregister(&packt_bus_type);
        err0:
            return status;
        }
        postcore_initcall(packt_init); // depends on busses and classes.
        ```

### Deep Dive in LDM
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
### Attribute group
```c
struct attribute_group {
    const char *name;
    umode_t (*is_visible)(struct kobject *,
                        struct attribute *, int);
    umode_t (*is_bin_visible)(struct kobject *,
                        struct bin_attribute *, int);
    struct attribute **attrs;
    struct bin_attribute **bin_attrs;
};
```
* if unamed: place all the attrs directly in the kobj directory when defining a group of attrbs.
* if named: a subdirectory will be created for the attribs, with dir name being the name of the attribute group.
* `is_visible()`: optional cb: return the permission associated with a specific attr in the group.
    * It will be called repeatedly for each(no-binary) attr in the group.
    * must then return the read/write permission of the attribute, or `0` if attr is not supposed to be accessed at all.
* `is_bin_visible()` : counter part of `is_visible()` for binary attrib.
* `attrs`: ptr to a `NULL` terminated list of attributes.
* `bin_attrs` : for bin attribs.
* Add/Remove apis: 
    ```c
    int sysfs_create_group(struct kobject *kobj,
                            const struct attribute_group *grp)
    void sysfs_remove_group(struct kobject * kobj,
                            const struct attribute_group * grp)
    ```   
* Example:
    ```c
    static struct kobj_attribute foo_attr = __ATTR(foo, 0660, attr_show, attr_store);
    static struct kobj_attribute bar_attr = __ATTR(bar, 0660, attr_show, attr_store);

    /* attrs is aa array of pointers to attributes */
    static struct attribute *demo_attrs[] = {
        &bar_foo_attr.attr,
        &bar_attr.attr,
        NULL,
    };

    static struct attribute_group my_attr_group = {
        .attrs = demo_attrs,
        /*.bin_attrs = demo_bin_attrs,*/
    };
    ```
* To create attributes in a single shot use `sysfs_create_group()`:
    ```c
    struct kobject *demo_kobj;
    int err;

    demo_kobj = kobject_create_and_add("demo", kernel_kobj);
    if (!demo_kobj) {
        pr_err("demo: demo_kobj registration failed.\n");
        return -ENOMEM;
    }

    err = sysfs_create_group(demo_kobj, &my_attr_group);
    if (err) {
        kobject_put(demo_kobj);
        return err;
    }
    ```
### Symbolic Links
* Drivers can create/remove symbolic links on existing kobjects (directories) using `sysfs_{create|remove}_link()` functions:
    ```c
    int sysfs_create_link(struct kobject * kobj,
                        struct kobject * target, char * name);

    void sysfs_remove_link(struct kobject * kobj, char * name);
    ```
    * The create function will create a symbolic link called `name` that points to the remote `target` kobject's sysfs entry.
    * A well-known example is devices appearing in both `/sys/bus` and `/sys/devices` since a bus controller is first a device on its own before exposing a bus.
 
    * However, note that any symbolic links that are created will be persistent (unless the system is rebooted), even after target removal. Thus, the driver must consider that when the associated device leaves the system or when the module is unloaded.

### Device-, driver-, bus- and class- related attributes
* All these frameworks proviodes attribute abstraction and file creation on top of low level kobjects.
* Each framework provides a framework-specific attribute data structure that encloses the default attribute and allows us to provide a custom show/store callback.
* Drivers
    ```c
    struct driver_attribute {
        struct attribute attr;
        ssize_t (*show)(struct device_driver *driver,
                        char *buf);
        ssize_t (*store)(struct device_driver *driver,
                        const char *buf, size_t count);
    };
    ```
* Classes
    ```c
    struct class_attribute {
        struct attribute attr;
        ssize_t (*show)(struct class *class,
                        struct class_attribute *attr, char *buf);
        ssize_t (*store)(struct class *class,
                        struct class_attribute *attr,
                        const char *buf, size_t count);
    };
    ```
* Bus
    ```c
    struct bus_attribute {
        struct attribute attr;
        ssize_t (*show)(struct bus_type *bus, char *buf);
        ssize_t (*store)(struct bus_type *bus,
                        const char *buf, size_t count);
    };
    ```
* Devices
    ```c
    struct device_attribute {
        struct attribute attr;
        ssize_t (*show)(struct device *dev,
                        struct device_attribute *attr,
                        char *buf);
        ssize_t (*store)(struct device *dev,
                        struct device_attribute *attr,
                        const char *buf, size_t count);
    };
    ```
* They can be dynamically allocated with `kzalloc()` and initialized by setting the
fields of their inner attribute elements and providing the appropriate callback
functions.
* each framework provides a set of macros to statically allocate, initialize, and assign a single instance of their respective attribute data structure.
* Bus
    ```c
    BUS_ATTR_RW(_name)
    BUS_ATTR_RO(_name)
    BUS_ATTR_WO(_name)
    ```
    * the resulting bus attribute variable is named `bus_attr_<_name>`.
* Drivers
    ```c
    DRIVER_ATTR_RW(_name)
    DRIVER_ATTR_RO(_name)
    DRIVER_ATTR_WO(_name)
    ```
    * `driver_attr_<_name>`
* Class
    ```c
    CLASS_ATTR_RW(_name)
    CLASS_ATTR_RO(_name)
    CLASS_ATTR_WO(_name)
    ```
    * `class_attr_<_name>`
* Device
    ```c
    DEVICE_ATTR(_name, _mode, _show, _store)
    DEVICE_ATTR_RW(_name)
    DEVICE_ATTR_RO(_name)
    DEVICE_ATTR_WO(_name)
    ```
    * `dev_attr_<_name>`
* Because all these macros are built on top of `__ATTR_RW`, `__ATTR_RO`, and
`__ATTR_WO`, they statically allocate and initialize a single instance of the
framework-specific attribute data structure and assume the show/store functions are named `<attribute_name>_show` and `<attribute_name>_store`.
* Creating files apis:
    ```c
    int device_create_file(struct device *device,
                            const struct device_attribute *entry);
    int driver_create_file(struct device_driver *driver,
                            const struct driver_attribute *attr);
    int bus_create_file(struct bus_type *bus, struct bus_attribute *);
    int class_create_file(struct class *class,
                            const struct class_attribute *attr)
    ```
* Example
    ```c
    int device_create_file(struct device *dev,
                        const struct device_attribute *attr)
    {
        [...]
        error = sysfs_create_file(&dev->kobj, &attr->attr);
        [...]
    }

    int class_create_file(struct class *cls,
                        const struct class_attribute *attr)
    {
        [...]
        error = sysfs_create_file(&cls->p->class_subsys.kobj, &attr->attr);
        return error;
    }

    int bus_create_file(struct bus_type *bus,
                        struct bus_attribute *attr)
    {
        [...]
        error = sysfs_create_file(&bus->p->subsys.kobj, &attr->attr);
        [...]
    }
    ```
* Removing files
    ```c
    void device_remove_file(struct device *device,
                            const struct device_attribute *entry);
    void driver_remove_file(struct device_driver *driver,
                            const struct driver_attribute *attr);
    void bus_remove_file(struct bus_type *, struct bus_attribute *);
    void class_remove_file(struct class *class,
                            const struct class_attribute *attr);
    ```
* Example device's implementation `drivers/base/core.c`
    ```c
    static ssize_t dev_attr_show(struct kobject *kobj,
                                struct attribute *attr,
                                char *buf)
    {
        struct device_attribute *dev_attr = to_dev_attr(attr);
        struct device *dev = kobj_to_dev(kobj);
        ssize_t ret = -EIO;
        
        if (dev_attr->show)
            ret = dev_attr->show(dev, dev_attr, buf);

        if (ret >= (ssize_t)PAGE_SIZE) {
            print_symbol("dev_attr_show: %s returned bad count\n",
                            (unsigned long)dev_attr->show);
        }
        
        return ret;
    }

    static ssize_t dev_attr_store(struct kobject *kobj,
                                struct attribute *attr,
                                const char *buf, size_t count)
    {
        struct device_attribute *dev_attr = to_dev_attr(attr);
        struct device *dev = kobj_to_dev(kobj);
        ssize_t ret = -EIO;
        if (dev_attr->store)
            ret = dev_attr->store(dev, dev_attr, buf, count);
        return ret;
    }

    static const struct sysfs_ops dev_sysfs_ops = {
        .show = dev_attr_show,
        .store = dev_attr_store,
    };
    ```
    * `to_dev_attr()`
    ```c
    #define to_dev_attr(_attr) \
        container_of(_attr, struct device_attribute, attr)
    ```
### Making a sysfs attribute poll- and select- compatible
* the main idea here is to allow the `poll()` or `select()` system calls to be used on a given attribute to passively wait for a change. 
* This change could be firmware becoming available, an alarm notification, or information that the attribute value has changed.
* driver must invoke `sysfs_notify()` to release any sleeping user.
    ```c
    void sysfs_notify(struct kobject *kobj, const char *dir,
                        const char *attr)
    ```
    * If the `dir` param is not `NULL`, it is used to find a subdirectory from within
    the directory of `kobj`, which contains the attribute (presumably created by
    `sysfs_create_group`).
    * This call will cause any polling process to wake up and
    process the event (which might be reading the new value, handling the alarm,
    and so on).
    * Example
    ```c
    static ssize_t store(struct kobject *kobj,
                        struct attribute *attr,
                        const char *buf, size_t len)
    {
        struct d_attr *da = container_of(attr, struct d_attr, attr);
        
        sscanf(buf, "%d", &da->value);
        pr_info("sysfs_foo store %s = %d\n", a->attr.name, a->value);

        if (strcmp(a->attr.name, "foo") == 0){
            foo.value = a->value;
            sysfs_notify(mykobj, NULL, "foo");
        } else if(strcmp(a->attr.name, "bar") == 0){
            bar.value = a->value;
            sysfs_notify(mykobj, NULL, "bar");
        }
            
        return sizeof(int);
    }
    ```
    * note that upon notification, `poll()` returns `POLLERR|POLLPRI` (as are flags, which users must request while invoking `poll()`), while `select()` returns the file descriptor, whether it is waiting for read, write, or exception events.

How the Wake-Up Mechanism Works
1. The Userspace Side (Waiting)
A userspace application sets up a file descriptor and waits for changes without burning CPU time:
    ```c
    int fd = open("/sys/kernel/demo/foo", O_RDONLY);
    struct pollfd pfd = {
        .fd = fd,
        .events = POLLPRI | POLLERR, // Listen for priority/exception events
    };

    while (1) {
        // This blocks until the kernel triggers a notification
        poll(&pfd, 1, -1); 

        // Once woken up, read the updated value
        lseek(fd, 0, SEEK_SET);
        read(fd, buf, sizeof(buf));
        printf("Value changed: %s\n", buf);
    }
    ```
2. The Kernel Side (sysfs_notify)
When a userspace process runs `echo 5 > /sys/kernel/demo/foo`, your `store()` function runs:
    1. It updates the internal value `(foo.value = a->value;)`.
    1. It calls `sysfs_notify(mykobj, NULL, "foo")`;.
    1. What `sysfs_notify` does internally:
        * It looks up the sysfs file node "foo" associated with mykobj.
        * It checks if any VFS/sysfs wait queues are listening on that file descriptor.
        * It wakes up those sleeping threads by signaling an event (typically `POLLPRI / priority` data ready).