#!/bin/sh

#export LD_ASSUME_KERNEL=2.3.98
export LD_LIBRARY_PATH=../lib:../lib/Tktable2.9/:$LD_LIBRARY_PATH
/usr/bin/valgrind $@
