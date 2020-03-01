#!/bin/bash
set -x

HOSTS="172.18.0.2 172.18.0.3"

# install in each node
for H in $HOSTS; do
    ssh $H "cd /usr/src/daloflow/mpich-3.3.2 && make install && ldconfig"
done

# execute cpi
cd /usr/src/daloflow/mpich/examples
mpicc -o cpi cpi.c
mpirun -np 2 --hosts 172.18.0.3,172.18.0.2 $(pwd)/cpi

