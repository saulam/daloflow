#!/bin/bash
#set -x


#*
#*  Copyright 2019-2020 Saul Alonso Monsalve, Felix Garcia Carballeira, Jose Rivadeneira Lopez-Bravo, Alejandro Calderon Mateos,
#*
#*  This file is part of DaLoFlow.
#*
#*  DaLoFlow is free software: you can redistribute it and/or modify
#*  it under the terms of the GNU Lesser General Public License as published by
#*  the Free Software Foundation, either version 3 of the License, or
#*  (at your option) any later version.
#*
#*  WepSIM is distributed in the hope that it will be useful,
#*  but WITHOUT ANY WARRANTY; without even the implied warranty of
#*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#*  GNU Lesser General Public License for more details.
#*
#*  You should have received a copy of the GNU Lesser General Public License
#*  along with WepSIM.  If not, see <http://www.gnu.org/licenses/>.
#*


#
# Params
#

 SIZES="32 64 128 256 512 1024"
 N_IMG_TRAIN="1000000"
 N_IMG_TEST="1000"


#
# Main
#

for S in $SIZES; do
    echo " * "$NI images of $S"x"$S"..."
    python3 mk_dataset.py --height $S --width $S --ntrain $N_IMG_TRAIN --ntest $N_IMG_TEST
done

