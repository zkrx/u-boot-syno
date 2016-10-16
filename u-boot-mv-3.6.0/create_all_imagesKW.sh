cd ..
mkdir images
cd images
mkdir DB-6281A-BP
mkdir DB-6192A-BP
mkdir RD-6281A
mkdir RD-6192A
mkdir DB-6180A-BP
mkdir DB-6280A-BP
mkdir DB-6282A-BP
cd DB-6281A-BP
mkdir LE
mkdir BE
cd LE
mkdir NAND
mkdir SPI
mkdir SPI+NAND
cd ../BE
mkdir NAND
mkdir SPI
mkdir SPI+NAND
cd ../../DB-6192A-BP
mkdir LE
mkdir BE
cd LE
mkdir NAND
mkdir SPI
mkdir SPI+NAND
cd ../BE
mkdir NAND
mkdir SPI
mkdir SPI+NAND
cd ../../RD-6281A
mkdir LE
mkdir BE
cd LE
mkdir NAND
mkdir SPI
mkdir SPI+NAND
cd ../BE
mkdir NAND
mkdir SPI
mkdir SPI+NAND
cd ../../RD-6192A
mkdir LE
mkdir BE
cd LE
mkdir SPI
cd ../BE
mkdir SPI
cd ../../DB-6180A-BP
mkdir LE
mkdir BE
cd LE
mkdir NAND
mkdir SPI
cd ../BE
mkdir NAND
mkdir SPI
cd ../../DB-6280A-BP
mkdir LE
mkdir BE
cd LE
mkdir NAND
mkdir SPI
mkdir SPI+NAND
cd ../BE
mkdir NAND
mkdir SPI
mkdir SPI+NAND
cd ../../DB-6282A-BP
mkdir LE
mkdir BE
cd LE
mkdir NAND
mkdir SPI
mkdir SPI+NAND
cd ../BE
mkdir NAND
mkdir SPI
mkdir SPI+NAND
cd ../../../u-boot-x.x.x

make mrproper
make db88f6281abp_config LE=1 NBOOT=1
make -s
cp u-boot-db88f6281abp_400db_nand.bin ../images/DB-6281A-BP/LE/NAND/
cp u-boot-db88f6281abp_400db_uart.bin ../images/DB-6281A-BP/LE/NAND/
cp u-boot-db88f6281abp ../images/DB-6281A-BP/LE/NAND/

make mrproper
make db88f6281abp_config LE=1 SPIBOOT=1
make -s
cp u-boot-db88f6281abp_400db_flash.bin ../images/DB-6281A-BP/LE/SPI
cp u-boot-db88f6281abp_400db_uart.bin ../images/DB-6281A-BP/LE/SPI
cp u-boot-db88f6281abp ../images/DB-6281A-BP/LE/SPI

make mrproper
make db88f6281abp_config LE=1 SPIBOOT=1 NAND=1
make -s
cp u-boot-db88f6281abp_400db_flash.bin ../images/DB-6281A-BP/LE/SPI+NAND
cp u-boot-db88f6281abp_400db_uart.bin ../images/DB-6281A-BP/LE/SPI+NAND
cp u-boot-db88f6281abp ../images/DB-6281A-BP/LE/SPI+NAND


make mrproper
make db88f6281abp_config BE=1 NBOOT=1
make -s
cp u-boot-db88f6281abp_400db_nand.bin ../images/DB-6281A-BP/BE/NAND
cp u-boot-db88f6281abp_400db_uart.bin ../images/DB-6281A-BP/BE/NAND
cp u-boot-db88f6281abp ../images/DB-6281A-BP/BE/NAND

make mrproper
make db88f6281abp_config BE=1 SPIBOOT=1
make -s
cp u-boot-db88f6281abp_400db_flash.bin ../images/DB-6281A-BP/BE/SPI
cp u-boot-db88f6281abp_400db_uart.bin ../images/DB-6281A-BP/BE/SPI
cp u-boot-db88f6281abp ../images/DB-6281A-BP/BE/SPI

make mrproper
make db88f6281abp_config BE=1 SPIBOOT=1 NAND=1
make -s
cp u-boot-db88f6281abp_400db_flash.bin ../images/DB-6281A-BP/BE/SPI+NAND
cp u-boot-db88f6281abp_400db_uart.bin ../images/DB-6281A-BP/BE/SPI+NAND
cp u-boot-db88f6281abp ../images/DB-6281A-BP/BE/SPI+NAND

make mrproper
make db88f6192abp_config LE=1 NBOOT=1
make -s
cp u-boot-db88f6192abp_200db619x_nand.bin ../images/DB-6192A-BP/LE/NAND
cp u-boot-db88f6192abp_200db619x_uart.bin ../images/DB-6192A-BP/LE/NAND
cp u-boot-db88f6192abp ../images/DB-6192A-BP/LE/NAND

make mrproper
make db88f6192abp_config LE=1 SPIBOOT=1
make -s
cp u-boot-db88f6192abp_200db619x_flash.bin ../images/DB-6192A-BP/LE/SPI
cp u-boot-db88f6192abp_200db619x_uart.bin ../images/DB-6192A-BP/LE/SPI
cp u-boot-db88f6192abp ../images/DB-6192A-BP/LE/SPI

make mrproper
make db88f6192abp_config LE=1 SPIBOOT=1 NAND=1
make -s
cp u-boot-db88f6192abp_200db619x_flash.bin ../images/DB-6192A-BP/LE/SPI+NAND
cp u-boot-db88f6192abp_200db619x_uart.bin ../images/DB-6192A-BP/LE/SPI+NAND
cp u-boot-db88f6192abp ../images/DB-6192A-BP/LE/SPI+NAND

make mrproper
make db88f6192abp_config BE=1 NBOOT=1
make -s
cp u-boot-db88f6192abp_200db619x_nand.bin ../images/DB-6192A-BP/BE/NAND
cp u-boot-db88f6192abp_200db619x_uart.bin ../images/DB-6192A-BP/BE/NAND
cp u-boot-db88f6192abp ../images/DB-6192A-BP/BE/NAND

make mrproper
make db88f6192abp_config BE=1 SPIBOOT=1
make -s
cp u-boot-db88f6192abp_200db619x_flash.bin ../images/DB-6192A-BP/BE/SPI
cp u-boot-db88f6192abp_200db619x_uart.bin ../images/DB-6192A-BP/BE/SPI
cp u-boot-db88f6192abp ../images/DB-6192A-BP/BE/SPI

make mrproper
make db88f6192abp_config BE=1 SPIBOOT=1 NAND=1
make -s
cp u-boot-db88f6192abp_200db619x_flash.bin ../images/DB-6192A-BP/BE/SPI+NAND
cp u-boot-db88f6192abp_200db619x_uart.bin ../images/DB-6192A-BP/BE/SPI+NAND
cp u-boot-db88f6192abp ../images/DB-6192A-BP/BE/SPI+NAND

make mrproper
make rd88f6281a_config LE=1 NBOOT=1
make -s
cp u-boot-rd88f6281a_400rd_nand.bin ../images/RD-6281A/LE/NAND
cp u-boot-rd88f6281a_400rd_uart.bin ../images/RD-6281A/LE/NAND
cp u-boot-rd88f6281a ../images/RD-6281A/LE/NAND

make mrproper
make rd88f6281a_config LE=1 SPIBOOT=1
make -s
cp u-boot-rd88f6281a_400rd_flash.bin ../images/RD-6281A/LE/SPI
cp u-boot-rd88f6281a_400rd_uart.bin ../images/RD-6281A/LE/SPI
cp u-boot-rd88f6281a ../images/RD-6281A/LE/SPI

make mrproper
make rd88f6281a_config LE=1 SPIBOOT=1 NAND=1
make -s
cp u-boot-rd88f6281a_400rd_flash.bin ../images/RD-6281A/LE/SPI+NAND
cp u-boot-rd88f6281a_400rd_uart.bin ../images/RD-6281A/LE/SPI+NAND
cp u-boot-rd88f6281a ../images/RD-6281A/LE/SPI+NAND

make mrproper
make rd88f6281a_config BE=1 NBOOT=1
make -s
cp u-boot-rd88f6281a_400rd_nand.bin ../images/RD-6281A/BE/NAND
cp u-boot-rd88f6281a_400rd_uart.bin ../images/RD-6281A/BE/NAND
cp u-boot-rd88f6281a ../images/RD-6281A/BE/NAND

make mrproper
make rd88f6281a_config BE=1 SPIBOOT=1
make -s
cp u-boot-rd88f6281a_400rd_flash.bin ../images/RD-6281A/BE/SPI
cp u-boot-rd88f6281a_400rd_uart.bin ../images/RD-6281A/BE/SPI
cp u-boot-rd88f6281a ../images/RD-6281A/BE/SPI

make mrproper
make rd88f6281a_config BE=1 SPIBOOT=1 NAND=1
make -s
cp u-boot-rd88f6281a_400rd_flash.bin ../images/RD-6281A/BE/SPI+NAND
cp u-boot-rd88f6281a_400rd_uart.bin ../images/RD-6281A/BE/SPI+NAND
cp u-boot-rd88f6281a ../images/RD-6281A/BE/SPI+NAND

make mrproper
make mrproper
make rd88f6192a_config LE=1 SPIBOOT=1
make -s
cp u-boot-rd88f6192a_200rd_flash.bin ../images/RD-6192A/LE/SPI
cp u-boot-rd88f6192a_200rd_uart.bin ../images/RD-6192A/LE/SPI
cp u-boot-rd88f6192a ../images/RD-6192A/LE/SPI

make mrproper
make rd88f6192a_config BE=1 SPIBOOT=1
make -s
cp u-boot-rd88f6192a_200rd_flash.bin ../images/RD-6192A/BE/SPI
cp u-boot-rd88f6192a_200rd_uart.bin ../images/RD-6192A/BE/SPI
cp u-boot-rd88f6192a ../images/RD-6192A/BE/SPI

make mrproper
make db88f6180abp_config LE=1 NBOOT=1
make -s
cp u-boot-db88f6180abp_200db_nand.bin ../images/DB-6180A-BP/LE/NAND/
cp u-boot-db88f6180abp_200db_uart.bin ../images/DB-6180A-BP/LE/NAND/
cp u-boot-db88f6180abp ../images/DB-6180A-BP/LE/NAND/

make mrproper
make db88f6180abp_config LE=1 SPIBOOT=1
make -s
cp u-boot-db88f6180abp_200db_flash.bin ../images/DB-6180A-BP/LE/SPI
cp u-boot-db88f6180abp_200db_uart.bin ../images/DB-6180A-BP/LE/SPI
cp u-boot-db88f6180abp ../images/DB-6180A-BP/LE/SPI

make mrproper
make db88f6180abp_config BE=1 NBOOT=1
make -s
cp u-boot-db88f6180abp_200db_nand.bin ../images/DB-6180A-BP/BE/NAND
cp u-boot-db88f6180abp_200db_uart.bin ../images/DB-6180A-BP/BE/NAND
cp u-boot-db88f6180abp ../images/DB-6180A-BP/BE/NAND

make mrproper
make db88f6180abp_config BE=1 SPIBOOT=1
make -s
cp u-boot-db88f6180abp_200db_flash.bin ../images/DB-6180A-BP/BE/SPI
cp u-boot-db88f6180abp_200db_uart.bin ../images/DB-6180A-BP/BE/SPI
cp u-boot-db88f6180abp ../images/DB-6180A-BP/BE/SPI

make mrproper
make db88f6280abp_config LE=1 NBOOT=1
make -s
cp u-boot-db88f6280abp_200db6280_nand.bin ../images/DB-6280A-BP/LE/NAND/
cp u-boot-db88f6280abp_200db6280_uart.bin ../images/DB-6280A-BP/LE/NAND/
cp u-boot-db88f6280abp ../images/DB-6280A-BP/LE/NAND/

make mrproper
make db88f6280abp_config LE=1 SPIBOOT=1
make -s
cp u-boot-db88f6280abp_200db6280_flash.bin ../images/DB-6280A-BP/LE/SPI
cp u-boot-db88f6280abp_200db6280_uart.bin ../images/DB-6280A-BP/LE/SPI
cp u-boot-db88f6280abp ../images/DB-6280A-BP/LE/SPI

make mrproper
make db88f6280abp_config LE=1 SPIBOOT=1 NAND=1
make -s
cp u-boot-db88f6280abp_200db6280_flash.bin ../images/DB-6280A-BP/LE/SPI+NAND
cp u-boot-db88f6280abp_200db6280_uart.bin ../images/DB-6280A-BP/LE/SPI+NAND
cp u-boot-db88f6280abp ../images/DB-6280A-BP/LE/SPI+NAND


make mrproper
make db88f6280abp_config BE=1 NBOOT=1
make -s
cp u-boot-db88f6280abp_200db6280_nand.bin ../images/DB-6280A-BP/BE/NAND
cp u-boot-db88f6280abp_200db6280_uart.bin ../images/DB-6280A-BP/BE/NAND
cp u-boot-db88f6280abp ../images/DB-6280A-BP/BE/NAND

make mrproper
make db88f6280abp_config BE=1 SPIBOOT=1
make -s
cp u-boot-db88f6280abp_200db6280_flash.bin ../images/DB-6280A-BP/BE/SPI
cp u-boot-db88f6280abp_200db6280_uart.bin ../images/DB-6280A-BP/BE/SPI
cp u-boot-db88f6280abp ../images/DB-6280A-BP/BE/SPI

make mrproper
make db88f6280abp_config BE=1 SPIBOOT=1 NAND=1
make -s
cp u-boot-db88f6280abp_200db6280_flash.bin ../images/DB-6280A-BP/BE/SPI+NAND
cp u-boot-db88f6280abp_200db6280_uart.bin ../images/DB-6280A-BP/BE/SPI+NAND
cp u-boot-db88f6280abp ../images/DB-6280A-BP/BE/SPI+NAND

make mrproper
make db88f6282abp_config LE=1 NBOOT=1
make -s
cp u-boot-db88f6282abp_400db_nand.bin ../images/DB-6282A-BP/LE/NAND/
cp u-boot-db88f6282abp_400db_uart.bin ../images/DB-6282A-BP/LE/NAND/
cp u-boot-db88f6282abp ../images/DB-6282A-BP/LE/NAND/

make mrproper
make db88f6282abp_config LE=1 SPIBOOT=1
make -s
cp u-boot-db88f6282abp_400db_flash.bin ../images/DB-6282A-BP/LE/SPI
cp u-boot-db88f6282abp_400db_uart.bin ../images/DB-6282A-BP/LE/SPI
cp u-boot-db88f6282abp ../images/DB-6282A-BP/LE/SPI

make mrproper
make db88f6282abp_config LE=1 SPIBOOT=1 NAND=1
make -s
cp u-boot-db88f6282abp_400db_flash.bin ../images/DB-6282A-BP/LE/SPI+NAND
cp u-boot-db88f6282abp_400db_uart.bin ../images/DB-6282A-BP/LE/SPI+NAND
cp u-boot-db88f6282abp ../images/DB-6282A-BP/LE/SPI+NAND


make mrproper
make db88f6282abp_config BE=1 NBOOT=1
make -s
cp u-boot-db88f6282abp_400db_nand.bin ../images/DB-6282A-BP/BE/NAND
cp u-boot-db88f6282abp_400db_uart.bin ../images/DB-6282A-BP/BE/NAND
cp u-boot-db88f6282abp ../images/DB-6282A-BP/BE/NAND

make mrproper
make db88f6282abp_config BE=1 SPIBOOT=1
make -s
cp u-boot-db88f6282abp_400db_flash.bin ../images/DB-6282A-BP/BE/SPI
cp u-boot-db88f6282abp_400db_uart.bin ../images/DB-6282A-BP/BE/SPI
cp u-boot-db88f6282abp ../images/DB-6282A-BP/BE/SPI

make mrproper
make db88f6282abp_config BE=1 SPIBOOT=1 NAND=1
make -s
cp u-boot-db88f6282abp_400db_flash.bin ../images/DB-6282A-BP/BE/SPI+NAND
cp u-boot-db88f6282abp_400db_uart.bin ../images/DB-6282A-BP/BE/SPI+NAND
cp u-boot-db88f6282abp ../images/DB-6282A-BP/BE/SPI+NAND

