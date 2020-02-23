#!/bin/bash
set -x

## /usr/src/horovod
cd /usr/src/horovod

# Download examples
apt-get install -y --no-install-recommends subversion && \
    svn checkout https://github.com/horovod/horovod/trunk/examples && \
    rm -rf /examples/.svn


