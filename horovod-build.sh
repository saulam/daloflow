#!/bin/bash
set -x

## /usr/src/daloflow/horovod
cd /usr/src/daloflow/horovod

python3 setup.py clean
CFLAGS="-march=native -mavx -mavx2 -mfma -mfpmath=sse" HOROVOD_WITHOUT_PYTORCH=1 HOROVOD_WITHOUT_MXNET=1 python3 setup.py bdist_wheel
pip3 install ./dist/horovod-*.whl

# Download examples
apt-get install -y --no-install-recommends subversion && \
    svn checkout https://github.com/horovod/horovod/trunk/examples && \
    rm -rf /examples/.svn


