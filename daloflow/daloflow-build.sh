#!/bin/bash
set -x


#
# MPICH
#

cd /usr/src/daloflow/
tar zxf mpich-3.3.2.tar.gz
cd /usr/src/daloflow/mpich-3.3.2

./configure --enable-orterun-prefix-by-default --disable-fortran
make -j $(nproc) all
make install
ldconfig 


#
# TENSORFLOW
#

cd /usr/src/daloflow/tensorflow

./configure
bazel build --config=opt //tensorflow/tools/pip_package:build_pip_package --action_env PYTHON_BIN_PATH=/usr/bin/python3 
./bazel-bin/tensorflow/tools/pip_package/build_pip_package /usr/src/daloflow/tensorflow/tensorflow_pkg
pip3 install /usr/src/daloflow/tensorflow/tensorflow_pkg/tensorflow-*.whl


#
# HOROVOD
#

cd /usr/src/daloflow/horovod

python3 setup.py clean
CFLAGS="-march=native -mavx -mavx2 -mfma -mfpmath=sse" python3 setup.py bdist_wheel
pip3 install ./dist/horovod-*.whl


