#!/bin/bash
set -x


#
# MPICH
#

cd /usr/src/daloflow/mpich
make install
ldconfig 


#
# TENSORFLOW
#

cd /usr/src/daloflow/tensorflow
pip3 install /usr/src/daloflow/tensorflow/tensorflow_pkg/tensorflow-*.whl


#
# HOROVOD
#

cd /usr/src/daloflow/horovod
pip3 install ./dist/horovod-*.whl

