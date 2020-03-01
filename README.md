# daloflow
Data locality on Tensorflow.


Scenario -> actions:
* First time   -> Clone project + Build the image
* Compile code -> Compile project
* Test example -> [Compile project] + Run project


Actions:
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
  * docker inspect -f '{{range .NetworkSettings.Networks}}{{.IPAddress}}{{end}}' $(docker ps|grep daloflow_node|cut -f1 -d' ') > machines
  * Example of working session:
    * docker container exec -it daloflow_node_1 ./daloflow-test.sh
    * docker container exec -it daloflow_node_1 mpirun -np 2 -machinefile machines /usr/src/daloflow/mpich/examples/cpi
  * docker-compose -f Dockercompose.yml down


Unsorted actions:
* Run docker in docker:
  * docker run -v /var/run/docker.sock:/var/run/docker.sock <other options>
* Clean all images (warning):
  * docker rmi -f $(docker images -q)

