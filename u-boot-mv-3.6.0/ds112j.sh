#!/bin/sh

make distclean; make DS112j_config BOOTROM=1 SPIBOOT=1 LE=1; make
