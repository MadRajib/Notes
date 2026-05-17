### Topics


### Intro
- **Bootloader**: it is part that starts the system and loads the OS kernel.
- It passes control from itself to the kernel using a data structure called a `device tree` also known as `flattened device tree` or `FDT`.
- Two main roles in LE(Linux Embedded):
    - initializing the system to a basic level
    - and loading the kernel.

Following a power-on or reset, the system is in very minimal state.
- The `DRAM` controller is not set up, so the main mem is not accessible.
- Likewise other interfaces are not configured, no storage access via `NAND flash`, `MultiMediaCard (MMC)` are not available.
- Only `SRAM` and boot `ROM` is present.

Interface between the bootloader and kernel are arch specific, but in each case:
1. First the bootloader has to pass a pointer to a structure containing info about the hardware configuration.
2. Second it has to pass a pointer to the kernel command line.`(a text string that handles controls the behavior of Linux)`

Once the kernel has loaded, there is no need of bootloader, all the mem it was using can be released.

A subsidiary job of it is to provide a maintenance mode for (usually throw serial console):
- updating boot config
- loading new boot images to memory
- maybe running diagnostics

## Boot Sequence
Some years ago:
- bootloader was placed in non-volatile mem at the reset vector of the processor.
- `NOR` flash mem was common at that time and since it can be mapped directly into the address space, it was ideal method of storage.
- reset vector addr - `0xfffffffc` at the top end of the area of the flash mem.
- After jump instruction ended up at start of bootloader code running in NOR flash mem, now it can initialize DRAM controller, so that main memory become available.
- Post that it can load the kernel from flash mem to the DRAM and transfer the control.

### Phase 1 - ROM Code
- The code that run immediately after a reset or power-on is stored on-chip in the SoC, the `ROM code`.
    - loaded into the chip when it was manufactured and hence the ROM code is proprietary and cannot be replaced by open source alternative.
- It does not include code to initialize the mem controller coz DRAM config are highly device specific.
- So `SRAM` is used which deon't requires a MMU.
- It can load a small chunk of code from one of several pre-programmed locations into SRAM.
- In SoCs where the SRAM is not large enough to load a full bootloader such as U-Boot there needs to be an intermediate loader `SPL (Secondary Program Loader)`.
- At the end of Phase SPL is present in the SRAM and the ROM code jumps to the beginning of that code.

### Phase 2 - SPL
- in this phase memory controller and other essential parts of the system in preparation for loading `Tertiary Program Loader (TPL)` into DRAM.
- functionality of SPL is limited by size of SRAM.
- SPL has filesystem drivers built into it, it can read well-known filenames such as `uboot.img` from the disk partition.
- Can print version info and other progress message in the console.
- It then loads `TPL` into DRAM. 
- At the end TPL is present in DRAM and the SPL can make jump to that area.

### Phase 3 - TPL
- At this point we are running full bootloader eg `U-boot`.
- There is a simple command line interface for maintenance tasks as well as a way to load the kernel automatically without user intervention.
- TPL loads the kernel into DRAM.
- We can also can append FDT or initial RAM disk to the image in DRAM.
- Before the end TPL needs to hand off control of the boot process to the kernel.

### Moving from bootloader to the kernel
Bootloader has to passes some basic info before handing the control over to kernel:
- The machine number
- basic details of the hardware that's been detected so far.
- the `kernel cmd line`
- optionally the location and size of a device tree binary
- Optionally the local of an initial RAM disk `(initramfs)`.

The bootloader and kernel require a binary representation of the `device tree` so it has to be compiled using the device tree compiler `(dtc)`.
- result is a file ending with `.dtb`
```bash
# scripts/dtc/dtc
$ dtc simpledts-1.dts -o simpledts-1.dtb
```
Bootloader can use device tree to initialize embedded SoC and its peripherals.

## U-Boot