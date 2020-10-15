#!/bin/bash
#set -x

SIZES="32 64 128 256 512 1024"
NIMG="1000000"


for S in $SIZES; do
for NI in $NIMG; do
	echo " * "$NI images of $S"x"$S"..."
	echo python3 horovod/examples/dump_dataset.py --height $S --width $S --ntrain $NIMG --ntest 1000
done
done

