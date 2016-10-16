# u-boot-syno
Synology's U-boot for the DS211j "modified" to load a uImage by tftp instead of flash.
This document describes a very hacky process I came up with to create and flash a new U-Boot on the DS211j.

# toolchain
Syno's U-Boot barely compiles. To avoid as many side effects as possible, I chose to create a symbolic link in /usr/local like so:
ln -s /opt/toolchains/arm-none-linux-gnueabi/ /usr/local/arm-none-linux-gnueabi

# image creation
```
cd u-boot-mv-3.4.4/
./ds211j.sh
```

Compilation will segfault when trying to use doimage. This is expected:
```
make: *** [Makefile:169: u-boot.bin] Segmentation fault (core dumped)
```

Sometimes the make process fails before the segfault. I have no idea why. Just relaunch the script if it happens. It should end with this output:
```
/usr/local/arm-none-linux-gnueabi/bin/arm-none-linux-gnueabi-objcopy --gap-fill=0xff -O binary u-boot u-boot.bin
cp -f u-boot.bin u-boot-DS211j.bin
./tools/doimage -T flash -D 0x600000 -E 0x670000 -R dramregs_x16cs0size128_A.txt u-boot-DS211j.bin u-boot-DS211j_x16cs0size128_flash.bin
cp -f u-boot u-boot-DS211j
cp -f u-boot.srec u-boot-DS211j.srec
make: *** [Makefile:169: u-boot.bin] Segmentation fault (core dumped)
make: *** Deleting file 'u-boot.bin'
make: *** Waiting for unfinished jobs....
```

We're building u-boot-mv-3.6.0 just to have a working doimage tool:
```
cd u-boot-mv-3.6.0/
./ds210j.sh
```

It doesn't matter if build fails. It will start by building the doimage tool which is all that matters to us.

And now we're able to execute the doimage.sh script that will create a working U-Boot binary in ../image/ :
```
./doimage.sh
```

Let's now proceed to the creation of the ready-to-flash image file
```
cd ../image
./create_img.sh <bin size> # where bin size is the size of the previously built u-boot-DS211j_x16cs0size128_flash.bin file in bytes
```

This will create two files:
- mtd0.dmp.uboot_new: whole flash image
- mtd0.dmp.part1: image of the first partition containing U-Boot and Kirkwood's firmware header

Use either one depending on your flashing tools.
Note that these images base themselves on an existing dump of the original flash content. You will have to use your own original dump if you're working on a different Synology device.

example:
```
cd u-boot-mv-3.4.4
./ds211j.sh
cd ../u-boot-mv-3.6.0
./ds210j.sh
./doimage.sh
cd ../image
ls -al # -rw-r--r-- 1 user user  466524 Oct 16 11:38 u-boot-DS211j_x16cs0size128_flash.bin
./create_img.sh 466524
```

# flashing
There is NO jtag access on the DS211j board (at least, I was unable to find one). The flash U-Boot image does not support cp, md.w, flashwrite to the NOR address space. However, I was able to corrupt the NOR memory by unintentionally performing a tftpboot on its address space, so I guess it's possible (though not ideal) to flash the NOR using tftpboot.

2 other solutions:
- manually load a custom crafted Linux kernel and use a USB stick with a custom rootfs (with all the flashcp stuff), that's what I did
- use a 3rd party flashing tool and access the NOR chip's pins directly. It's a standard m25p32 SPI flash chip. I unsoldered it and used a RPi with a custom DTB to restore the flash to its original content after screwing up (using the SPI pins on the main RPi header).

If you screw up at this stage, the DS211j won't boot (you will hear the fan go wild) and you will need physical access to the flash chip to restore its content. Proceed at your own risks.
