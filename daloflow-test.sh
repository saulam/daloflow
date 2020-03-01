#!/bin/bash
set -x


#
# MPICH
#

# execute cpi
cd /usr/src/daloflow/mpich/examples
mpicc -o cpi cpi.c
mpirun -np 2 --hosts 172.18.0.3,172.18.0.2 $(pwd)/cpi


#
# HOROVOD
#

# horovodrun --verbose -np 2 -H localhost:2 python training.py

