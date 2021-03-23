#!/bin/bash
set -x

time ./hdfs-cp.sh list.txt /mnt/local-storage/tmp/
