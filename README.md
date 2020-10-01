# daloflow
DAta LOcality on tensorFLOW.

First time:
* To clone from github
  * git clone https://github.com/saulam/daloflow.git
* Postclone project:
  * cd daloflow
  * chmod +x ./daloflow.sh
  * ./daloflow.sh postclone
* Pre-requisites:
  * ./daloflow.sh prerequisites
* Build the docker image:
  * ./daloflow.sh image

Work session:
* Start work session:
  * ./daloflow.sh start <number of container>
* Run your applications. For example:
  * ./daloflow.sh mpirun <np> "python3 ./horovod/examples/tensorflow2_mnist.py"
  * ...
* Stop work session:
  * ./daloflow.sh stop

Some options for debugging:
* ./daloflow.sh status
* ./daloflow.sh test
* ./daloflow.sh bash <id container, from 1 up to nc>

