# daloflow
DAta LOcality on tensorFLOW.

First time:
* To clone from github
  * git clone https://github.com/saulam/daloflow.git
* Postclone project:
  * cd daloflow
  * chmod +x ./daloflow.sh
  * ./daloflow.sh postclone
* Pre-requisites (if docker + docker-compose is not installed):
  * ./daloflow.sh prerequisites
* Build the docker image:
  * ./daloflow.sh image

Work session (in a single node):
* Start work session:
  * Single node:
    * ./daloflow.sh start <number of containers>
  * Several nodes:
    * ./daloflow.sh swarm-start <number of containers>
* Run your applications. For example:
  * ./daloflow.sh mpirun <np> "python3 ./horovod/examples/tensorflow2_keras_mnist.py"
  * ./daloflow.sh mpirun <np> "python3 ./horovod/examples/tensorflow2_mnist.py"
  * ...
* Stop work session:
  * Single node:
    * ./daloflow.sh stop
  * Several nodes:
    * ./daloflow.sh swarm-stop

Some options for debugging:
* ./daloflow.sh status
* ./daloflow.sh test
* ./daloflow.sh bash <id container, from 1 up to nc>

