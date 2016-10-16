cp ../u-boot-mv-3.4.4/u-boot-DS211j.bin .
./tools/doimage -T flash -D 0x600000 -E 0x670000 -R dramregs_x16cs0size128_A.txt u-boot-DS211j.bin u-boot-DS211j_x16cs0size128_flash.bin
cp u-boot-DS211j_x16cs0size128_flash.bin ../image/   
