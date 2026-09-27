### Driver Registration
* __Devices__: Registering a device inserts it into the device list managed by its parent bus driver.

* __Drivers__: Registering a device driver inserts it into the driver list managed by the bus driver.

    Examples:

    * A USB device driver gets added to the USB controller's driver list.

    * An SPI device driver gets added to the SPI controller's driver list.

* The Registration & Binding Process

    * __driver_register()__: The underlying core kernel function that adds a generic `struct device_driver` to a bus’s list of drivers.

    * __Device List Traversal__: Once registered, the driver core walks through the bus's list of unmatched devices.

    * __Bus Matching__: For each unmatched device, the kernel calls the bus's `match()` callback to check if the new driver can handle the device.

    * __Binding__: When `match()` returns true, the device and driver are bound together (triggering the driver's `.probe()` callback).

* In practice, driver developers rarely call `driver_register()` directly. Bus subsystems provide high-level wrappers following the standard `{bus_name}_register_driver()` naming convention:

    | Bus SubsystemDriver | Registration Function |
    | ------|------|
    | USB | usb_register_driver() |
    | SPI | spi_register_driver() |
    | I2C | i2c_register_driver() |
    | PCI | pci_register_driver() |
    | Platform | platform_driver_register() |

* Lifecycle & Best Practices
    * __Init/Exit Placement__: Driver registration and unregistration should be handled in the module's initialization (`init`) and exit (`exit`) functions, triggered when the module is loaded (`insmod`) or unloaded (`rmmod`).
    * Kernel buses provide shorthand helper macros following the pattern:
        ```c
        module_{bus_name}_driver(__driver_struct);
        ```
    * These macros automatically expand into standard `init` and `exit` entry points while executing `{bus_name}_register_driver()` and `{bus_name}_unregister_driver()` internally.
    * Common Helper Macros Across Buses

    | Subsystem / Bus | Macro| Underlying Action | 
    |-----|-----|-----|
    | Platform | `module_platform_driver(drv)` | Expands to `platform_driver_register()` / `platform_driver_unregister()` |
    |I2C | `module_i2c_driver(drv)` | Expands to `i2c_add_driver()` / `i2c_del_driver()`|
    | SPI | `module_spi_driver(drv)` | Expands to `spi_register_driver()` / `spi_unregister_driver()` |
    | PCI | `module_pci_driver(drv)` | Expands to `pci_register_driver()` / `pci_unregister_driver()`|
    | USB | `module_usb_driver(drv)` | Expands to `usb_register_driver()` / `usb_deregister_driver()` |

* Example
    ```c

    static struct platform_driver mypdrv = {
        .probe = my_pdrv_probe,
        .remove = my_pdrv_remove,
        .driver = {
            .name = KBUILD_MODNAME,
            .owner = THIS_MODULE,
        },
    };

    // Previously
    static int __init my_driver_init(void) {
        return platform_driver_register(&mypdrv);
    }

    static void __exit my_driver_exit(void) {
        platform_driver_unregister(&mypdrv);
    }

    module_init(my_driver_init);
    module_exit(my_driver_exit);

    // Macro Approach
    // Replaces the entire init/exit boilerplate code above
    module_platform_driver(mypdrv);
    ```

    ### Exposing the supported devices in the driver