### Intro
1. Axis released thir first linux powered network camera (1999) 
1. TiVo their first DVR with linux (1999)

### Topics
- [When not to choose linux](#when-not-to-choose-linux)
- [Four Elements of embedded Linux](#four-elements-of-embedded-linux)
- [Choosing the hardware](#selecting-hardware-for-embedded-linux)
- [Optaining the hardware](#optaning-the-hardware)

### When not to choose Linux
- when you are limited with hardware resources since linux requires 32 bit processor and lots more memory.
- Board bringup requires detail knowlege of linux and how it relates to your hardware. Do you have the skillset ?
- Is your system realtime ? Linux can handle realtime tasks if one knows where to pay attention.
- Will your code require regulatory apporval ? (Required for medical, aerospace, and so on) Look for other solution already in the field.

### Four elements of embedded Linux
1. **Tooltain**: the compier and other tools needed to build your code for your target device.
2. **Bootloader**: One who initialises the board and loads the linux kernel.
3. **Kernel**: One who manages the system resources handles hardware interfaces etc.
4. **Root Filesystem**: Libs and binaries than run once kernel has completed its initialisation.

There is a fifth elemented : collection of programs for your specific embedded application.

### Selecting Hardware for embedded linux
Look for ?
1. **CPU** arch that is supported by the kernel. eg (Arm, MIPS PowerPC, x86 etc). Almost all of them come with **MMU** except some group eg **Microcontroller Linx or uClinux** (ARC, Blackfin, MicroBlaze and Nois arch).
2. **RAM** specially amount or ram. 16 MiB is good amount. Even possible to run at 4 MiB with tough optimisations.
3. **Non-volatile** storage, usually flash memory. 8 MiB is enought for a simple device (simple small router or webcam). Linux has extensive support for NOR and NAND flash chips, SD cards, EMMC chips, USB flash memory and so on.
4. **Serial Port**: Preferably UART-based serial port, makes board bring-up, debugging and developement much easier.
5. **Means to load software when starting from scratch**. eg JTAG interfaces. Modern SoCs have the ability to load the boot code directly from removable media or serial interfaces like UART or USB.

### Optaning the hardware
1. We will be using beagle bone black for the learning purpose.
2. For QEMU :
```bash
$ qemu-system-arm -machine vexpress-a9 -m 256M -drive file=rootfs.ext4,sd -net nic -net use -kernel zImage -dtb  vexpress-v2p-ca9.dtb -append "console=ttyAMA0,115200 root=/dev/mmcblk0" -serial stdio -net nic,model=lan9118 -net tap,ifname=tap0
```
* `-machine vexpress -a9`: Creates an emulation of an Arm Versatile Express development
board with a Cortex-A9 processor.
* `-m 256M`: Populates it with 256 MB of RAM.
* `-drive file=rootfs.ext4,sd`: Connects the SD interface to the local rootfs.ext4 file, which contains a filesystem image.
* `-kernel zImage`: Loads the Linux kernel from the local file named zImage.
* `-dtb vexpress-v2p-ca9.dtb`: Loads the device tree from the local vexpress-v2p-ca9.dtb
file.
* `-append "…"`: Appends the string in quotes as the kernel command line.
* `-serial stdio`: Connects the serial port to the terminal that launched QEMU so that you can log on to the emulated machine via the serial console.
* `-net nic,model=lan9118`: Creates a network interface.
* `-net tap,ifname=tap0`: Connects the network interface to the virtual network interface tap0

To Configure the host side of the network you need the `tunctl` command from the
`User Mode Linux (UML)` project.

```bash
$ sudo apt-get intall uml-utilites
$ sudo tunctl -u $(whoami) -t tap0
```

This creates a network interface name `tap0` that is connected to the network controller in the emulated QEMU machine.