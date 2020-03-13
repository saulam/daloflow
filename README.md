# daloflow
Data locality on Tensorflow.

First time:
* Pre-requisites:
  * make prerequisites
* Clone project:
  * make clone
* Build the image:
  * [edit Dockerfile if needed]
  * [update options if needed]
  * make image

Work session:
* Start work session:
  * make start NC=2
* [when source is modified] Compile project:
  * docker container exec -it daloflow_node_1 make build
* Run example (examples of possible executions):
  * docker container exec -it daloflow_node_1 make test
  * docker container exec -it daloflow_node_1 mpirun -np 2 -machinefile machines_mpi -bind-to none -map-by slot python3 ./horovod/examples/tensorflow2_mnist.py
* [when not longer be used] Stop work session:
  * make stop


