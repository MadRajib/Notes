
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

- Before you can build an external module, you need to have a complete and precompiled kernel source tree.
-  There are two ways to obtain a prebuilt kernel version:
    - __Building it by yourself__: This can be used for both native and cross-compilation. Using a build system such as Yocto or Buildroot may help.
    - Installing the **linux-headers-*** package from the distribution package feed: This applies only for x86 native compilations unless your embedded target runs a Linux distribution that maintains a package feed (such as Raspbian).

Note:
 > It must be noted that is it not possible to build a built-in kernel module with out-of-tree building. \
 > The reason is that building a Linux kernel module out of tree requires a prebuilt or prepared kernel

**Native and out-of-tree module compiling**

```bash
sudo apt update
sudo apt install linux-headers-$(uname -r)

#  $(uname -r) corresponds to the
kernel version in use
```

- To load and unload the build kernel
```bash

$ make

..
$ ls
helloworld.c  helloworld.ko  helloworld.mod.c  helloworld.mod.o  helloworld.o  Makefile  module
s.order  Module.symvers

$ sudo insmod  helloworld.ko
$ sudo rmmod helloworld
$ dmesg
[...]
[308342.285157] Hello world initialization!
[308372.084288] Hello world exit
```

**Out-of-tree module cross-compiling**

- For building a cross-compiling an out-of-tree kernel module, two variables kernel **make** cmds needs
    - **ARCH** : target architecture
    - **CROSS_COMPILE**: cross-compiler prefix
- When using a build system such as Yocto, the Linux kernel is first cross-compiled as a dependency before it starts cross
compiling the module.
- **KERNEL_SRC** variable is automatically exported by Yocto for kernel module recipies.
- It is set to the value of **STAGING_KENREL_DIR** within the **module.bbclass** class inherited by all kernle module recipies.
- what changes between native compilation and cross-compilation of an out-of-tree kernel module is the final
make command,
```bash
# for 32 bit
make ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf-
# for 64 bit
make ARCH=aarch64 CROSS_COMPILE=aarch64-linux-gnu-

# assuming the cross-compiled kernel source path has been specified in the makefile.
```

### In-tree building

- In-tree module building requires dealing with an additional file, **Kconfig**, which allows us to expose the module features in the configuration menu.

- Add the following content to the Kconfig file of that directory:
```bash
config PACKT_MYCDEV
    tristate "Our packtpub special Character driver"
    default m
    help
      Say Y here to support /dev/mycdev char device.
      The /dev/mycdev is used to access packtpub.
```
- In Makefile
```bash
obj-$(CONFIG_PACKT_MYCDEV)   += mychardev.o

#  the .o file name must match the exact name of the .c file.
```

- In order to have your module built as a loadable kernel module, add the following line to your defconfig board in the **arch/arm/configs** directory:
```bash
CONFIG_PACKT_MYCDEV=m
```
- You can also run **menuconfig** to select it from the UI, run make to build the kernel, and then make modules to build modules (including yours).
- To make the driver built-in, just replace m with y:
```bash
CONFIG_PACKT_MYCDEV=y
```
- Once configured, you can build the kernel with **make**, and build modules with **make modules**.
- Modules included in the kernel source tree are installed in **/lib/modules/$(uname -r)/kernel/**


### Handling module parameters

- Similar to a user program, a kernel module can accept arguments from the command line.
- For this first we need to declare the variables that will hold the values of cmd-line arguments and use the **module_param()** macro on each of these.
- The macro is defined in **include/linux/moduleparam.h**

```c
module_param(name, type, perm);

# name: The name of the variable used as the parameter.
# type: The parameter's type (bool, charp, byte, short, ushort, int, uint, long, and ulong), where charp stands for character pointer.
# perm: This represents the /sys/module/<module>/parameters/<param> file permissions. Some of them are S_IWUSR, S_IRUSR, S_IXUSR, S_IRGRP, S_WGRP, and S_IRUGO, where the following applies:
#    - S_I is just a prefix.
#    - R = read, W = write, and X = execute.
#    - USR = user, GRP = group, and UGO = user, group, and others.
```

- **MODULE_PARM_DESC** can be used on a per-parameter basis to describe the module parameters.
```c
#include <linux/moduleparam.h>
[...]
static char *mystr = "hello";
static int myint = 1;
static int myarr[3] = {0, 1, 2};

module_param(myint, int, S_IRUGO);
module_param(mystr, charp, S_IRUGO);
module_param_array(myarr, int,NULL, S_IWUSR|S_IRUSR);

MODULE_PARM_DESC(myint,"this is my int variable");
MODULE_PARM_DESC(mystr,"this is my char pointer variable");
MODULE_PARM_DESC(myarr,"this is my array of int");

static int foo()
{
    pr_info("mystring is a string: %s\n",
             mystr);
    pr_info("Array elements: %d\t%d\t%d",
             myarr[0], myarr[1], myarr[2]);
    return myint;
}
```

- we could have used modinfo prior to loading the module in order to display a description of parameters supported by the module:
```bash
$ modinfo ./helloworld-params.ko
filename:       /home/jma/work/tutos/sources/helloworld/./helloworld-params.ko
license:      GPL
author:       John Madieu <john.madieu@gmail.com>
srcversion:   BBF43E098EAB5D2E2DD78C0
depends:        
vermagic:     4.4.0-93-generic SMP mod_unload modversions
parm:         myint:this is my int variable (int)
parm:         mystr:this is my char pointer variable (charp)
parm:         myarr:this is my array of int (array of int)
```

- To load the module and feed our parameter
```bash
# insmod hellomodule-params.ko mystring="packtpub" myint=15 myArray=1,2,3
```
- It is also possible to find and edit the current values for the parameters of a loaded module from **Sysfs** in **/sys/module/<name>/parameters**.
    - In that directory, there is one file per parameter, containing the parameter value.
    - These parameter values can be changed if the associated files have write permissions (which depends on the module code).
    
    ```bash
    echo 0 > /sys/module/usbcore/parameters/authorized_default
    ```
- Static kernel modules can also read parameters using **CONFIG_CMDLINE**
```bash
[initial command line ...] my_module.param=value
```

- To trigger a function when parameter value is changed:
```c
static int enable_feature;

static int param_set_enable(const char *val, const struct kernel_param *kp)
{
    int ret = param_set_int(val, kp);
    if (ret)
        return ret;

    if (enable_feature)
        setup_feature();
    else
        teardown_feature();

    return 0;
}

static const struct kernel_param_ops param_ops_enable = {
    .set = param_set_enable,
    .get = param_get_int,
};

module_param_cb(enable_feature, &param_ops_enable, &enable_feature, 0644);

// echo 0 > /sys/module/my_module/parameters/enable_feature
``` 

### Dealing with symbol exports and module dependencies

- The Linux kernel exposes two macros that can be used to export functions and variables. These are the following:
    - **EXPORT_SYMBOL(symbolname)**: This macro exports a function or variable to all modules.
    - **EXPORT_SYMBOL_GPL(symbolname)**: This macro exports a function or variable only to GPL modules.

- **EXPORT_SYMBOL()** instructs the Kbuild mechanism
to include the symbol passed as an argument in the global list of kernel symbols.

