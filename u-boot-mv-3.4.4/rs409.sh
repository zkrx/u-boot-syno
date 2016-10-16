#!/bin/sh

make distclean; make RS409_config BOOTROM=1 SPIBOOT=1 LE=1; make
