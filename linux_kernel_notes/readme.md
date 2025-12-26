
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