#!/bin/bash
set -x

cd /usr/src/daloflow/
tar zxf mpich-3.3.2.tar.gz
cd /usr/src/daloflow/mpich-3.3.2

./configure --enable-orterun-prefix-by-default --disable-fortran
make -j $(nproc) all
make install
ldconfig 

cd /usr/src/daloflow/
