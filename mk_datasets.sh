#!/bin/bash
#set -x

#
# Params
#

#SIZES="32 64 128 256 512 1024"
 SIZES="512 1024"
 N_IMG_TRAIN="1000000"
 N_IMG_TEST="1000"


#
# Main
#

for S in $SIZES; do
    echo " * "$NI images of $S"x"$S"..."
    python3 horovod/examples/dump_dataset.py --height $S --width $S --ntrain $N_IMG_TRAIN --ntest $N_IMG_TEST
done

