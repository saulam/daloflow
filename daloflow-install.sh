#!/bin/bash
set -x


#
# MPICH
#

cd /usr/src/daloflow/mpich-3.3.2
make install
ldconfig 


#
# TENSORFLOW
#

cd /usr/src/daloflow/tensorflow
pip3 install /tmp/tensorflow_pkg/tensorflow-*.whl


#
# HOROVOD
#

cd /usr/src/daloflow/horovod
pip3 install ./dist/horovod-*.whl

