# daloflow
Data locality on Tensorflow.

First time:
* Pre-requisites:
  * sudo ./daloflow-prerequisites.sh
* Clone project:
  * git clone https://github.com/saulam/daloflow.git
  * git submodule update --init --recursive
* Build the image:
  * [edit Dockerfile if needed]
  * [update options if needed]
  * docker image build -t daloflow:1 .

Work session:
* Start work session:
  * ./daloflow-start.sh 2
* [when source is modified] Compile project:
  * docker container exec -it daloflow_node_1 ./daloflow-build.sh
* Run example (examples of possible executions):
  * docker container exec -it daloflow_node_1 ./daloflow-test.sh
  * docker container exec -it daloflow_node_1 mpirun -np 2 -machinefile machines_mpi -bind-to none -map-by slot python3 ./horovod/examples/tensorflow2_mnist.py
* [when not longer be used] Stop work session:
  * ./daloflow-stop.sh


Unsorted actions:
* Run docker in docker:
  * docker run -v /var/run/docker.sock:/var/run/docker.sock <other options>
* Clean all images (warning):
  * docker rmi -f $(docker images -q)
* Change daloflow path:
  * find ./ -type f -exec sed -i 's/\/usr\/src\/daloflow/yourPath/g' {} \;
* Compile project:
  * docker run --network host -v $(pwd):/usr/src/daloflow -it daloflow:1 bash
  * ./mpich-build.sh		
  * ./tensorflow-build.sh
  * ./horovod-build.sh	
  * exit


ISSUES:
* tensorflow-build.sh request python3 path (ignore the 'enviromental¡ configuration)
* compilar código y ejemplos de mpich, tensorflow, etc.

