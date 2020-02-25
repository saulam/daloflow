#!/bin/bash
set -x

## /usr/src/horovod
cd /usr/src/horovod

python setup.py clean
python setup.py bdist_wheel
pip3install ./dist/horovod-*.whl

# Download examples
apt-get install -y --no-install-recommends subversion && \
    svn checkout https://github.com/horovod/horovod/trunk/examples && \
    rm -rf /examples/.svn


