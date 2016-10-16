cd ..
mkdir images
cd images
mkdir DB-6281A-BP
mkdir DB-6192A-BP
mkdir RD-6281A
mkdir RD-6192A
cd DB-6281A-BP
mkdir LE
mkdir BE
cd LE
mkdir NAND
mkdir SPI
cd ../BE
mkdir NAND
mkdir SPI
cd ../../DB-6192A-BP
mkdir LE
mkdir BE
cd LE
mkdir NAND
mkdir SPI
cd ../BE
mkdir NAND
mkdir SPI
cd ../../RD-6281A
mkdir LE
mkdir BE
cd LE
mkdir NAND
mkdir SPI
cd ../BE
mkdir NAND
mkdir SPI
cd ../../RD-6192A
mkdir LE
mkdir BE
cd LE
mkdir SPI
cd ../BE
mkdir SPI
cd ../../../u-boot-3.4.*

make mrproper
make db88f6281abp_config LE=1 NBOOT=1
make -s
cp u-boot-db88f6281abp.bin ../images/DB-6281A-BP/LE/NAND/
cp u-boot-db88f6281abp_400_nand.bin ../images/DB-6281A-BP/LE/NAND/
cp u-boot-db88f6281abp_400_uart.bin ../images/DB-6281A-BP/LE/NAND/

make mrproper
make db88f6281abp_config LE=1 SPIBOOT=1
make -s
cp u-boot-db88f6281abp.bin ../images/DB-6281A-BP/LE/SPI
cp u-boot-db88f6281abp_400_flash.bin ../images/DB-6281A-BP/LE/SPI
cp u-boot-db88f6281abp_400_uart.bin ../images/DB-6281A-BP/LE/SPI
make mrproper
make db88f6281abp_config BE=1 NBOOT=1
make -s
cp u-boot-db88f6281abp.bin ../images/DB-6281A-BP/BE/NAND
cp u-boot-db88f6281abp_400_nand.bin ../images/DB-6281A-BP/BE/NAND
cp u-boot-db88f6281abp_400_uart.bin ../images/DB-6281A-BP/BE/NAND
make mrproper
make db88f6281abp_config BE=1 SPIBOOT=1
make -s
cp u-boot-db88f6281abp.bin ../images/DB-6281A-BP/BE/SPI
cp u-boot-db88f6281abp_400_flash.bin ../images/DB-6281A-BP/BE/SPI
cp u-boot-db88f6281abp_400_uart.bin ../images/DB-6281A-BP/BE/SPI

make mrproper
make db88f6192abp_config LE=1 NBOOT=1
make -s
cp u-boot-db88f6192abp.bin ../images/DB-6192A-BP/LE/NAND
cp u-boot-db88f6192abp_200db_nand.bin ../images/DB-6192A-BP/LE/NAND
cp u-boot-db88f6192abp_200db_uart.bin ../images/DB-6192A-BP/LE/NAND
make mrproper
make db88f6192abp_config LE=1 SPIBOOT=1
make -s
cp u-boot-db88f6192abp.bin ../images/DB-6192A-BP/LE/SPI
cp u-boot-db88f6192abp_200db_flash.bin ../images/DB-6192A-BP/LE/SPI
cp u-boot-db88f6192abp_200db_uart.bin ../images/DB-6192A-BP/LE/SPI
make mrproper
make db88f6192abp_config BE=1 NBOOT=1
make -s
cp u-boot-db88f6192abp.bin ../images/DB-6192A-BP/BE/NAND
cp u-boot-db88f6192abp_200db_nand.bin ../images/DB-6192A-BP/BE/NAND
cp u-boot-db88f6192abp_200db_uart.bin ../images/DB-6192A-BP/BE/NAND
make mrproper
make db88f6192abp_config BE=1 SPIBOOT=1
make -s
cp u-boot-db88f6192abp.bin ../images/DB-6192A-BP/BE/SPI
cp u-boot-db88f6192abp_200db_flash.bin ../images/DB-6192A-BP/BE/SPI
cp u-boot-db88f6192abp_200db_uart.bin ../images/DB-6192A-BP/BE/SPI

make mrproper
make rd88f6281a_config LE=1 NBOOT=1
make -s
cp u-boot-rd88f6281a.bin ../images/RD-6281A/LE/NAND
cp u-boot-rd88f6281a_400_nand.bin ../images/RD-6281A/LE/NAND
cp u-boot-rd88f6281a_400_uart.bin ../images/RD-6281A/LE/NAND
make mrproper
make rd88f6281a_config LE=1 SPIBOOT=1
make -s
cp u-boot-rd88f6281a.bin ../images/RD-6281A/LE/SPI
cp u-boot-rd88f6281a_400_flash.bin ../images/RD-6281A/LE/SPI
cp u-boot-rd88f6281a_400_uart.bin ../images/RD-6281A/LE/SPI
make mrproper
make rd88f6281a_config BE=1 NBOOT=1
make -s
cp u-boot-rd88f6281a.bin ../images/RD-6281A/BE/NAND
cp u-boot-rd88f6281a_400_nand.bin ../images/RD-6281A/BE/NAND
cp u-boot-rd88f6281a_400_uart.bin ../images/RD-6281A/BE/NAND
make mrproper
make rd88f6281a_config BE=1 SPIBOOT=1
make -s
cp u-boot-rd88f6281a.bin ../images/RD-6281A/BE/SPI
cp u-boot-rd88f6281a_400_flash.bin ../images/RD-6281A/BE/SPI
cp u-boot-rd88f6281a_400_uart.bin ../images/RD-6281A/BE/SPI

make mrproper
make mrproper
make rd88f6192a_config LE=1 SPIBOOT=1
make -s
cp u-boot-rd88f6192a.bin ../images/RD-6192A/LE/SPI
cp u-boot-rd88f6192a_200rd_flash.bin ../images/RD-6192A/LE/SPI
cp u-boot-rd88f6192a_200rd_uart.bin ../images/RD-6192A/LE/SPI
make mrproper
make rd88f6192a_config BE=1 SPIBOOT=1
make -s
cp u-boot-rd88f6192a.bin ../images/RD-6192A/BE/SPI
cp u-boot-rd88f6192a_200rd_flash.bin ../images/RD-6192A/BE/SPI
cp u-boot-rd88f6192a_200rd_uart.bin ../images/RD-6192A/BE/SPI
