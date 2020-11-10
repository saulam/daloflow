<html>
 <h1 align="center">daloflow: <br>DAta LOcality on tensorFLOW</h1>
 <br>
</html>

## Get daloflow and setup initial stuff:
1. To clone from github
   * git clone https://github.com/saulam/daloflow.git
   * cd daloflow
   * chmod +x ./daloflow.sh
   * ./daloflow.sh init cpu
2. IF docker + docker-compose is not installed THEN install pre-requisites:
   * ./daloflow.sh prerequisites
3. Build the docker image:
   * ./daloflow.sh build
  
## Typical daloflow work session:
1. Start work session:
   * Single node:
     * ./daloflow.sh start <number of containers>
   * Several nodes:
     * ./daloflow.sh swarm-start <number of containers>
2. Run the applications, for example for NP=2:
   * ./daloflow.sh mpirun 2 "python3 ./do_tf2kp_mnist.py"
   * ...
3. Stop work session:
   * ./daloflow.sh stop

### Some additional options for debugging:
* ./daloflow.sh status
* ./daloflow.sh bash <id container, from 1 up to NC>


## Authors
* :technologist: Saúl Alonso Monsalve
* :technologist: Félix García-Carballeira
* :technologist: José Rivadeneira López-Bravo 
* :technologist: Alejandro Calderón Mateos
