#!/bin/bash
set -x

## /usr/src/tensorflow
cd /usr/src/daloflow/tensorflow
./configure
bazel build --config=opt //tensorflow/tools/pip_package:build_pip_package --action_env PYTHON_BIN_PATH=/usr/bin/python3 
./bazel-bin/tensorflow/tools/pip_package/build_pip_package /tmp/tensorflow_pkg
pip3 install /tmp/tensorflow_pkg/tensorflow-*.whl

