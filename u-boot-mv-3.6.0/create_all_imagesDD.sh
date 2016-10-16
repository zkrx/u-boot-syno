TOPDIR=`pwd`
rm -rf ../images
mkdir -p ../images/DB-78XX0-A-BP/LE/NAND
mkdir -p ../images/DB-78XX0-A-BP/BE
mkdir -p ../images/DB-78200-A-BP/LE/NAND
mkdir -p ../images/DB-78200-A-BP/LE/CV
mkdir -p ../images/DB-78200-A-BP/LE/DUAL_LINUX
mkdir -p ../images/DB-78200-A-BP/BE/CV
mkdir -p ../images/DB-78200-A-BP/BE/DUAL_LINUX
mkdir -p ../images/RD-78200-A-AMC/LE
mkdir -p ../images/DB-88F86323-BP/LE/NAND/DUAL_LINUX_SHARED_MEM
mkdir -p ../images/DB-88F86323-BP/LE/SPI/DUAL_LINUX

make mrproper
make db78XX0_config LE=1
make -s 
cp u-boot-db78XX0.bin ../images/DB-78XX0-A-BP/LE/
cp u-boot-db78XX0.srec ../images/DB-78XX0-A-BP/LE/
cp u-boot-db78XX0 ../images/DB-78XX0-A-BP/LE/

make mrproper
make db78XX0_config LE=1 NBOOT=1
make -s 
make nboot
cp u-boot-db78XX0.bin ../images/DB-78XX0-A-BP/LE/NAND
cp u-boot-db78XX0.srec ../images/DB-78XX0-A-BP/LE/NAND
cp u-boot-db78XX0 ../images/DB-78XX0-A-BP/LE/NAND
cp nboot.bin ../images/DB-78XX0-A-BP/LE/NAND

make mrproper
make db78XX0_config BE=1
make -s
cp u-boot-db78XX0.bin ../images/DB-78XX0-A-BP/BE/
cp u-boot-db78XX0.srec ../images/DB-78XX0-A-BP/BE/
cp u-boot-db78XX0 ../images/DB-78XX0-A-BP/BE/

make mrproper
make db78200_MP_config LE=1
make -s
cp u-boot-db78200_MP.bin ../images/DB-78200-A-BP/LE/CV
cp u-boot-db78200_MP.srec ../images/DB-78200-A-BP/LE/CV
cp u-boot-db78200_MP ../images/DB-78200-A-BP/LE/CV

make mrproper
make db78200_MP_config LE=1 NBOOT=1
make -s
make nboot
cp u-boot-db78200_MP.bin ../images/DB-78200-A-BP/LE/NAND
cp u-boot-db78200_MP.srec ../images/DB-78200-A-BP/LE/NAND
cp u-boot-db78200_MP ../images/DB-78200-A-BP/LE/NAND
cp nboot.bin ../images/DB-78200-A-BP/LE/NAND

make mrproper
make db78200_MP_config LE=1 LNX=1
make -s
cp u-boot-db78200_MP.bin ../images/DB-78200-A-BP/LE/DUAL_LINUX
cp u-boot-db78200_MP.srec ../images/DB-78200-A-BP/LE/DUAL_LINUX
cp u-boot-db78200_MP ../images/DB-78200-A-BP/LE/DUAL_LINUX

make mrproper
make db78200_MP_config BE=1
make -s
cp u-boot-db78200_MP.bin ../images/DB-78200-A-BP/BE/CV
cp u-boot-db78200_MP.srec ../images/DB-78200-A-BP/BE/CV
cp u-boot-db78200_MP ../images/DB-78200-A-BP/BE/CV

make mrproper
make db78200_MP_config BE=1 LNX=1
make -s
cp u-boot-db78200_MP.bin ../images/DB-78200-A-BP/BE/DUAL_LINUX
cp u-boot-db78200_MP.srec ../images/DB-78200-A-BP/BE/DUAL_LINUX
cp u-boot-db78200_MP ../images/DB-78200-A-BP/BE/DUAL_LINUX

make mrproper
make rd78200_MP_AMC_config LE=1 SPIBOOT=1
make -s
cp u-boot-rd78200_MP_AMC.bin ../images/RD-78200-A-AMC/LE/
cp u-boot-rd78200_MP_AMC.srec ../images/RD-78200-A-AMC/LE/
cp u-boot-rd78200_MP_AMC ../images/RD-78200-A-AMC/LE/


make mrproper
make db88f632x_config SPIBOOT=1 LE=1 LNX=1
make -s
cp u-boot-db88f632x.bin ../images/DB-88F86323-BP/LE/SPI/DUAL_LINUX
cp u-boot-db88f632x.srec ../images/DB-88F86323-BP/LE/SPI/DUAL_LINUX
cp u-boot-db88f632x ../images/DB-88F86323-BP/LE/SPI/DUAL_LINUX

make mrproper
make db88f632x_config NBOOT=1 LE=1
make -s
make nboot
cp u-boot-db88f632x.bin ../images/DB-88F86323-BP/LE/NAND
cp u-boot-db88f632x.srec ../images/DB-88F86323-BP/LE/NAND
cp u-boot-db88f632x ../images/DB-88F86323-BP/LE/NAND
cp nboot.bin ../images/DB-88F86323-BP/LE/NAND

make mrproper
make db88f632x_config NBOOT=1 LE=1 LNX_SHARED=1
make -s
make nboot
cp u-boot-db88f632x.bin ../images/DB-88F86323-BP/LE/NAND/DUAL_LINUX_SHARED_MEM
cp u-boot-db88f632x.srec ../images/DB-88F86323-BP/LE/NAND/DUAL_LINUX_SHARED_MEM
cp u-boot-db88f632x ../images/DB-88F86323-BP/LE/NAND/DUAL_LINUX_SHARED_MEM
cp nboot.bin ../images/DB-88F86323-BP/LE/NAND/DUAL_LINUX_SHARED_MEM
