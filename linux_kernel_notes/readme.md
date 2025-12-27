
## Understanding Linux Kernel Module Basic Concepts

### Intro to concept of modules
- Some functions are inbuilt to kernel -> static modules (built in module)
- It is part of final kernel kernel image.
- Some features like device drivers can be compiled as loadable modules.
- can be considered as plugins which can be loaded n unloaded on demand.
- To support module loading, it can be enabled in kernel using
    ```bash
    CONFIG_MODULES=y
    ```
- To support unloading of a loadable module using:
    ```bash
    CONFIG_MODULE_UNLOAD=y
    ```
    - Without this we won't be able to unload any module
- Kernel may prevent unloading of modules which may break things, even when asked to do so.
- Coz the kernel keeps a reference count of module usage so that it
knows whether a module is currently in use or not. If the kernel believes it is unsafe to remove a module, it will not.
- We can change this behavior via:
    ```bash
    MODULE_FORCE_UNLOAD=y
    ```
    - The preceding option allows us to force module unload.

###  Module skeleton
```c
#include <linux/module.h>
#include <linux/init.h>

static int __init helloworld_init(void) {
    pr_info("Hello world initialization!\n");
    return 0;
}
static void __exit helloworld_exit(void) {
    pr_info("Hello world exit\n");
}

module_init(helloworld_init);
module_exit(helloworld_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("John Madieu <john.madieu@gmail.com>");
MODULE_DESCRIPTION("Linux kernel module skeleton");
```
- In the preceding skeleton, headers are specific to the Linux kernel, hence the use of linux/xxx.h.
- The module.h header file
is mandatory for all kernel modules
- and init.h is needed for the __init and __exit macros.


**Module entry and exit points**

- The minimal requirement of a kernel module is an initialization method. This is must.
-  If the module can be built as a
loadable module, then the exit method must be provided as well.
- modules can be loaded using : **modprobe** or **insmod**
- modules can be loaded using : **rmmod** or **modprobe -r**
- first entry point is called during loading and exit point is called during unloading.


- The helloworld_init and helloworld_exit functions can be given any name.
- The only thing that is actually mandatory is to
identify them as the corresponding initialization and exit functions, passing them as parameters to the **module_init()** and
**module_exit()** macros.

- init or exit methods are invoked only once, whatever the number of devices currently handled by the module, provided the module is a device driver.

- **__init** and **__exit** are kernel macros, defined in include/linux/init.h, as shown here:
    ```c
    #define __init      __section(.init.text)
    #define __exit      __section(.exit.text)
    ```
- The __init keyword tells the linker to place the symbols (variables or functions) they prefix in a dedicated section in the resulting kernel object file.
- This section is known in advance to the kernel and freed when the module is loaded and the initialization function has finished.
- Note: This applies only to built-in modules, not to loadable ones.
- The kernel will run the initialization function of the driver for the first time during its boot sequence.- Since the driver cannot be unloaded, its initialization function will never be called again until the next reboot.
- There is no need to keep references on this initialization function anymore.
- It is the same for the **__exit** keyword and the exit method, whose corresponding code is omitted when the module is compiled statically into the kernel or when module unloading support is not enabled because, in both cases, the exit function is never called. 
- **__exit** has no effect on loadable modules.
- these macros directs compiler to put the code in **.init.text** and **.exit.text** sections, respectively.


**Module information and metadata**
- A kernel module uses its **.modinfo** section to store information
about the module.
- Any **MODULE_** macro will update the content of this section.
- The real underlying macro provided by the kernel to add an entry to the module info section is **MODULE_INFO(tag, info)**.
    - THis adds generic info of the tag = "info" form. eg
    ```c
    MODULE_INFO(my_field_name, "What easy value");
    ```
- **MODULE_LICENSE()**
    - Defines how the module’s source code is licensed
    - Informs the kernel about the license type of the module
    - Affects module behavior and symbol access
    - Non-GPL compatible license:
        - Cannot access symbols exported using **EXPORT_SYMBOL_GPL()**
        - Can only access symbols exported with **EXPORT_SYMBOL()**
    - Loading a non-GPL module:
        - Taints the kernel
        - Indicates closed-source or untrusted code
        - Community support may be unavailable
    - A module without **MODULE_LICENSE()**:
        - Treated as non-open-source
        - Also taints the kernel
    - Supported licenses are listed in:
        - include/linux/module.h
- **MODULE_AUTHOR()**
    - declares the module's author(s)
    - can have more than one name
    ```c
    MODULE_AUTHOR("John Madieu <john.madieu@foobar.com>");
    MODULE_AUTHOR("Lorem Ipsum <l.ipsum@foobar.com>");
    ```
- **MODULE_DESCRIPTION()**
    - briefly describes what the module does:
    ```c
    MODULE_DESCRIPTION("Hello, world! Module").
    ```
- You can dump the content of the **.modeinfo** section of a kernel module using the **objdump -d -j .modinfo** command on
the given module.
- For a cross-compiled module, you should use **$(CORSS_COMPILE)objdump** instead.

### Building a Linux kernel module

Two Solution exits:
1.  Out-of-tree building: when the code is outside the kernel src tree.
    - Building a module this way does not allow integration into the kernel configuration/compilation process, and the module needs to be built separately.
    -  It must be noted that with this solution, the module cannot be statically linked in the final kernel image – that is, it cannot be built in.
    - Out-of-tree compilation only allows loadable kernel modules to be produced.
1. Inside kernel Tree:
    - Allows to upstream your code.
    - Allows tatically linked and loadable kernel module.

### Linux kernel build system

- LK has its own build system **kbuild**.
- Allows to configure the LK and compile it based on the config that has been done.
- Relies mainly on 3 files:
    1. Kconfig    // feature selection
    1. Kbuild or  // compilation rules
    1. Makefile

**The Kbuild or Makefile files**

- If both files exist, only Kbuild will be
used.
- eg: using a make file to build a module
    ```bash
    make -C $KERNEL_SRC M=$(shell pwd) [target]
    ```
    - **$KERNEL_SRC** refers to the path of the prebuilt kernel directory
    - **-C $KERNEL_SRC** instructs make to change into the specified directory when executing and change back when finished.
    - **M=$(shell pwd)** in-structs the kernel build system to move back into this directory to find the module that is being built.
        - The value given to **M** is the absolute path of the directory where the module sources (or the associated Kbuild file) are located.
    - **[target]** corresponds to the subset of the make targets available when building an external module.
        - **modules**: This is the default target for external modules. It has the same functionality as if no target was specified.
        - **modules_install**: This installs the external module(s). The default location is **/lib/modules/<kernel_release>/extra/**. This path can be overridden.
        - **clean**: This removes all generated files (in the module directory only).
- To tell the build system what obj files to build or link together:
```bash
obj-<X> := <module_name>.o
```
    - the kernel build system will build **<module_name>.o** from **<module_name>.c** or **<module_name>.S**
    - and after linking, it will result in the **<module_name>.ko** kernel loadable module or will be part of the single-file kernel image.
    - **<X>** can be either **y**, **m**, or left blank(wont be built).
- However, the **obj-$(CONFIG_XXX)** pattern is often used, where **CONFIG_XXX** is a kernel configuration option, set or not,
during the kernel configuration process.

```bash
obj-$(CONFIG_MYMODULE) += mymodule.o

# $(CONFIG_MYMODULE) evaluates to either y, m, or nothing (blank), according to its value during the kernel configuration
```
- For multiple src file
```bash
<module_name>-y := <file1>.o <file2>.o

# eg.
obj-m := foo.o bar.o
```
- If foo.o and bar.o are made of source files other than foo.c and bar.c
```bash
obj-m := foo.o bar.o
foo-y := foo1.o foo2.o . . .
bar-y := bar1.o bar2.o bar3.o . . .
```
- Other flags
```bash
ccflags-y := -I$(src)/include
ccflags-y += -I$(src)/src/hal/include
ldflags-y := -T$(src)foo_sections.lds

obj-<X> += somedir/
# the kernel build system should go into the directory named somedir and look for any Makefile or Kbuild files inside
```
- Eg
```bash
# kbuild part of makefile
obj-m := helloworld.o
#the following is just an example
#ldflags-y := -T foo_sections.lds
# normal makefile
KERNEL_SRC ?= /lib/modules/$(shell uname -r)/build
all default: modules
install: modules_install
modules modules_install help clean:
    $(MAKE) -C $(KERNEL_SRC) M=$(shell pwd) $@
```

### Out-of-tree building