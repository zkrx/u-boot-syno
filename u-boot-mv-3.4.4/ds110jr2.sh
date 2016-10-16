#!/bin/sh

make distclean; make DS110jr2_config BOOTROM=1 SPIBOOT=1 LE=1; make
