#!/bin/bash
set -x

## /usr/src/tensorflow
cd /usr/src/daloflow/tensorflow
./configure
bazel build --config=opt //tensorflow/tools/pip_package:build_pip_package --action_env PYTHON_BIN_PATH=/usr/bin/python3 
pip3 install /usr/src/tensorflow/tensorflow-*.whl

