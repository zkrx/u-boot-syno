#!/bin/sh

make distclean; make DS409_config BOOTROM=1 SPIBOOT=1 LE=1; make
