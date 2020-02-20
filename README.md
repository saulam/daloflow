# daloflow
Data locality on Tensorflow.


Clone Project
* git clone https://github.com/saulam/daloflow.git
* git submodule update --init --recursive

Prepare docker
* docker image build -t daloflow:0.1 .
* docker run -v $(pwd):/usr/src/daloflow -it bash daloflow:0.1

