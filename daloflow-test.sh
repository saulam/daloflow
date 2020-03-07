#!/bin/bash
set -x


#
# MPICH
#

# execute cpi
cd /usr/src/daloflow/mpich/examples
mpicc -o cpi cpi.c
mpirun -np 2 -machinefile /usr/src/daloflow/machines_mpi $(pwd)/cpi


#
# HOROVOD
#

mpirun -np 2 -machinefile machines_mpi -bind-to none -map-by slot python3 ./horovod/examples/tensorflow2_mnist.py
# horovodrun --verbose -np 2 -hostfile machines_horovod  python3 ./horovod/examples/tensorflow2_mnist.py

