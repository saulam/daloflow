
## Compile and Install Tensorflow + Open MPI + Horovod
# /usr/src/mpich
# /usr/src/tensorflow
# /usr/src/horovod

RUN cd /usr/src/daloflow/tensorflow \
    ./configure \
    bazel build --config=opt //tensorflow/tools/pip_package:build_pip_package \
    pip3 install /usr/src/tensorflow/tensorflow-*.whl

RUN cd /usr/src/daloflow/mpich && \
    ./configure --enable-orterun-prefix-by-default && \
    make -j $(nproc) all && \
    make install && \
    ldconfig 

#TODO: horovod

