#!/bin/sh

make distclean; make RS411_config BOOTROM=1 SPIBOOT=1 LE=1; make
