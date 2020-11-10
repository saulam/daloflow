<html>
 <h1 align="center">daloflow: <br>DAta LOcality on tensorFLOW</h1>
 <br>
</html>

## Getting daloflow and initial setup:
1. To clone from github:
```bash
 git clone https://github.com/saulam/daloflow.git
 cd daloflow
 chmod +x ./daloflow.sh
 ./daloflow.sh init cpu
``` 
2. IF docker + docker-compose is not installed THEN please install pre-requisites:
```bash
 ./daloflow.sh prerequisites
```
3. To build the docker image:
```bash
 ./daloflow.sh build
```
  
## Typical daloflow work session:
1. To start a new work session:
 * Using a Single node:
```bash
     ./daloflow.sh start <number of containers>
```
 * Using Several nodes:
```bash
     ./daloflow.sh swarm-start <number of containers>
```
2. To run the applications, for example for NP=2:
```bash
 ./daloflow.sh mpirun 2 "python3 ./do_tf2kp_mnist.py"
 ...
```
3. To stop work session:
```bash
 ./daloflow.sh stop
```


### Some additional options for debugging:
```bash
 ./daloflow.sh status
 ./daloflow.sh bash <id container, from 1 up to NC>
```


## Authors
* :technologist: Saúl Alonso Monsalve
* :technologist: Félix García-Carballeira
* :technologist: José Rivadeneira López-Bravo 
* :technologist: Alejandro Calderón Mateos
