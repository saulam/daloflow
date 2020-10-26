# daloflow
DAta LOcality on tensorFLOW.

### Get daloflow and setup initial stuff:
1. To clone from github
   * git clone https://github.com/saulam/daloflow.git
   * cd daloflow
   * chmod +x ./daloflow.sh
   * ./daloflow.sh postclone
2. IF docker + docker-compose is not installed THEN install pre-requisites:
   * ./daloflow.sh prerequisites
3. Build the docker image:
   * ./daloflow.sh build
  
### Typical daloflow work session:
1. Start work session:
   * Single node:
     * ./daloflow.sh start <number of containers>
   * Several nodes:
     * ./daloflow.sh swarm-start <number of containers>
2. Run your applications. For example:
   * ./daloflow.sh mpirun <np> "python3 ./horovod/examples/tensorflow2_keras_mnist.py"
   * ./daloflow.sh mpirun <np> "python3 ./horovod/examples/tensorflow2_mnist.py"
   * ...
3. Stop work session:
   * Single node:
     * ./daloflow.sh stop
   * Several nodes:
     * ./daloflow.sh swarm-stop

### Some additional options for debugging:
* ./daloflow.sh status
* ./daloflow.sh test
* ./daloflow.sh bash <id container, from 1 up to nc>

