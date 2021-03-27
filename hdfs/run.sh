#!/bin/bash
set -x

rm /mnt/local-storage/tmp/texto.txt.*
time ./hdfs-cp.sh list.txt /mnt/local-storage/tmp/

