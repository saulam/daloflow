# daloflow
Data locality on Tensorflow.


Scenario -> actions:
* First time   -> Pre-requisites + Clone project + Build the image
* Compile code -> Compile project
* Test example -> [Compile project] + Run project


Actions:
* Pre-requisites:
  * Install Docker:
    * sudo apt-get update
    * sudo apt-get upgrade
    * sudo apt-get install  curl apt-transport-https ca-certificates software-properties-common
    * curl -fsSL https://download.docker.com/linux/ubuntu/gpg | sudo apt-key add -
    * sudo add-apt-repository "deb [arch=amd64] https://download.docker.com/linux/ubuntu $(lsb_release -cs) stable"
    * sudo apt-get update
    * sudo apt install docker-ce
  * Install Docker-Compose:
    * pip3 install docker-compose

* Clone project:
  * git clone https://github.com/saulam/daloflow.git
  * git submodule update --init --recursive

* Build the image:
  * [edit Dockerfile if needed]
  * [update options if needed]
  * docker image build -t daloflow:1 .

* Compile project:
  * docker run --network host -v $(pwd):/usr/src/daloflow -it daloflow:1 bash
  * ./mpich-build.sh		
  * ./tensorflow-build.sh
  * ./horovod-build.sh	
  * exit

* Test example:
  * docker-compose -f Dockercompose.yml up -d --scale node=2
  * for C in $(docker ps -q); do docker container exec -it $C ./daloflow-install.sh ; done
  * docker inspect -f '{{range .NetworkSettings.Networks}}{{.IPAddress}}{{end}}' $(docker ps|grep daloflow_node|cut -f1 -d' ') > machines_mpi
  * docker inspect -f '{{range .NetworkSettings.Networks}}{{.IPAddress}}{{end}}' $(docker ps|grep daloflow_node|cut -f1 -d' ') | sed 's/.*/& slots=1/g' > machines_horovod
  * Example of working session:
    * docker container exec -it daloflow_node_1 ./daloflow-test.sh
    * docker container exec -it daloflow_node_1     mpirun           -np 2 -machinefile machines_mpi   /usr/src/daloflow/mpich/examples/cpi
    * docker container exec -it daloflow_node_1     mpirun           -np 2 -machinefile machines_mpi -bind-to none -map-by slot python3 ./horovod/examples/tensorflow2_mnist.py
    * docker container exec -it daloflow_node_1 horovodrun --verbose -np 2 -hostfile machines_horovod  python3 ./horovod/examples/tensorflow2_mnist.py
  * docker-compose -f Dockercompose.yml down


Unsorted actions:
* Run docker in docker:
  * docker run -v /var/run/docker.sock:/var/run/docker.sock <other options>
* Clean all images (warning):
  * docker rmi -f $(docker images -q)
* Change daloflow path:
  * find ./ -type f -exec sed -i 's/\/usr\/src\/daloflow/yourPath/g' {} \;


ISSUES:
* tensorflow-build.sh request python3 path (ignore the 'enviromental¡ configuration)

