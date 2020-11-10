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
<html>
 <table>
  <tr>
  <td>
</html>

A new single node work session:
```bash
   ./daloflow.sh start 4
   ./daloflow.sh mpirun 2 "python3 ./do_tf2kp_mnist.py"
   : ...
   ./daloflow.sh stop
```

For example, with "./daloflow.sh start" four container is spin-up in one node, the current one (NC=4).
Then, do_tf2kp_mnist.py was executed with 2 process (NP=2, only two containers are used).

<html>
  </td>
  <td>
</html>

A new work session using several nodes:
```bash
   ./daloflow.sh swarm-start 4
   ./daloflow.sh mpirun 2 "python3 ./do_tf2kp_mnist.py"
   : ...
   ./daloflow.sh stop
```

For example, with "./daloflow.sh swarm-start" a container is spin-up in four nodes (NC=4, one container per node).
Then, do_tf2kp_mnist.py was executed with 2 process (NP=2) on the first two nodes.

<html>
  </td>
  </tr>
 </table>
</html>


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
