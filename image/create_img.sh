if [ $# -ne 1 ]; then
    echo "$0: usage: $0 <bin size in bytes>"
    exit 1
fi

echo "Creating total flash image (mtd0.dmp.uboot_new)"
cp u-boot-DS211j_x16cs0size128_flash.bin mtd0.dmp.uboot_new
dd if=mtd0.dmp.orig of=mtd0.dmp.uboot_new seek=$1 skip=$1 bs=1

echo "Exporting partition #1 (mtd0.dmp.part1)"
dd if=mtd0.dmp.uboot_new of=mtd0.dmp.part1 bs=1 count=524288

echo "Diffing input/output (there should be NO diff output!)"
diff <(xxd u-boot-DS211j_x16cs0size128_flash.bin) <(xxd -l $1 mtd0.dmp.part1)

echo "Image ready!"
