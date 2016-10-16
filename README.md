# u-boot-syno
Synology's U-boot for the DS211j "modified" to load a uImage by tftp instead of flash

# toolchain
Syno's U-Boot barely compiles. To avoid as many side effects as possible, I personnally create a symbolic link in /usr/local like so:
ln -s /opt/toolchains/arm-none-linux-gnueabi/ /usr/local/arm-none-linux-gnueabi

# compilation
cd u-boot-mv-3.4.4/
./ds211j.sh

Compilation will segfault when trying to use doimage. This is expected:
make: *** [Makefile:169: u-boot.bin] Segmentation fault (core dumped)

We're building u-boot-mv-3.6.0 just to have a working doimage tool:
cd u-boot-mv-3.6.0/
chmod 777 ds210j.sh
./ds210j.sh

And now we're able to execute the doimage.sh script that will create a working U-Boot binary in ../image/ :
./doimage.sh

