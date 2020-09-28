# daloflow
Data locality on Tensorflow.

First time:
* Pre-requisites:
  * ./daloflow.sh prerequisites
* Clone project:
  * ./daloflow.sh clone
* Build the image:
  * [update Dockerfile if needed]
  * ./daloflow.sh image

Work session:
* Start work session:
  * ./daloflow.sh start <number of container>
* [when tensorflow or horovod source code is modified] Compile the new binary:
  * ./daloflow.sh build
* Run example (examples of possible executions):
  * ./daloflow.sh mpirun <np> "python3 ./horovod/examples/tensorflow2_mnist.py"
* [when some debug if needed] Test
  * ./daloflow.sh status
  * ./daloflow.sh test
  * ./daloflow.sh bash <id container, from 1 up to nc>
* [when not longer be used] Stop work session:
  * ./daloflow.sh stop

