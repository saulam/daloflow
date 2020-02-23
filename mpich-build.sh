#!/bin/bash
set -x

## /usr/src/mpich (From Packed Source)
cd /usr/src/daloflow/
tar zxf mpich-3.3.2.tar.gz
cd /usr/src/daloflow/mpich-3.3.2
./configure --enable-orterun-prefix-by-default --disable-fortran
make -j $(nproc) all
make install
ldconfig 

## /usr/src/mpich (from GitHub)
#cd /usr/src/daloflow/mpich
#autoreconf -i
#./configure --enable-orterun-prefix-by-default
#make -j $(nproc) all
#make install
#ldconfig 

