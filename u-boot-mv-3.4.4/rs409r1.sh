#!/bin/sh

make distclean; make RS409r1_config BOOTROM=1 SPIBOOT=1 LE=1; make
