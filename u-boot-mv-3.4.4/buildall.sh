#!/bin/sh

./ds409slim.sh; 
cp u-boot-DS409slim_x16cs0size128_flash.bin uboot_ds409slim_bin; 

./ds209.sh; 
cp u-boot-DS209_x16cs01size256_flash.bin uboot_ds209_bin;

./ds109.sh;
cp u-boot-DS109_x16cs0size128_flash.bin uboot_ds109_bin

./ds409.sh;
cp u-boot-DS409_x16cs01size256_flash.bin uboot_ds409_bin

./rs409.sh;
cp u-boot-RS409_x16cs01size256_flash.bin uboot_rs409_bin

./ds509.sh;
cp u-boot-DS509_x16cs01size256_flash.bin uboot_ds509_bin
