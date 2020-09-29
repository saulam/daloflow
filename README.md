# daloflow
Data locality on Tensorflow.

First time:
* Pre-requisites:
  * ./daloflow.sh prerequisites
* Clone project:
  * ./daloflow.sh clone
* Build the image (from Dockerfile):
  * ./daloflow.sh image

Work session:
* Start work session:
  * ./daloflow.sh start <number of container>
  * ./daloflow.sh build
* Run your applications. For example:
  * ./daloflow.sh mpirun <np> "python3 ./horovod/examples/tensorflow2_mnist.py"
  * ...
* Stop work session:
  * ./daloflow.sh stop

Some options for debugging:
* ./daloflow.sh status
* ./daloflow.sh test
* ./daloflow.sh bash <id container, from 1 up to nc>

