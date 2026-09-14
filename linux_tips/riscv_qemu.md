# Qemu Setup

### Install the required build tools, QEMU, and libraries on your host system
```bash
sudo apt update && sudo apt install -y build-essential qemu-system-x86 qemu-system-arm qemu-system-misc gdb git bc bison flex libssl-dev libelf-dev
```

### Install cross compiler 
```bash
sudo apt install gcc-riscv64-linux-gnu
```

### Download and Configure the Linux Kernel
```bash
git clone --depth 1 https://github.com/torvalds/linux.git
```

### Build kernel
```bash
cd linux/
make ARCH=riscv CROSS_COMPILE=riscv64-linux-gnu- defconfig
make ARCH=riscv CROSS_COMPILE=riscv64-linux-gnu- -j$(nproc)
```

### Setuo BusyBox
```bash
git clone https://git.busybox.net/busybox && cd busybox
make ARCH=riscv CROSS_COMPILE=riscv64-linux-gnu- defconfig
make ARCH=riscv CROSS_COMPILE=riscv64-linux-gnu- menuconfig

# Enable Static lib
# **Settings → Build Options**, and enable **Build static binary (no shared libs)**
# Disable tc: for compilation errors
# **Networking Utilities** → uncheck **tc**
```

### Build Busybox
```bash
make ARCH=riscv CROSS_COMPILE=riscv64-linux-gnu- -j$(nproc)
make ARCH=riscv CROSS_COMPILE=riscv64-linux-gnu- install
```

### Build rootfs
```bash
mkdir rootfs
cd rootfs/
cp -av ../_install/* .

cat << 'EOF' > init
#!/bin/sh
mount -t proc none /proc
mount -t sysfs none /sys
echo "Welcome to custom Linux kernel development!"
exec /bin/sh
EOF

chmod +x init

find . -print0 | cpio --null -ov --format=newc | gzip -9 > ../initramfs.cpio.gz
```

### Load linux with Qemu and rootfs
```bash
qemu-system-riscv64 -machine virt -nographic -kernel arch/riscv/boot/Image -initrd ../busybox/riscv-initramfs.cpio.gz -append "console=ttyS0"
```