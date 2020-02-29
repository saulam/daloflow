# daloflow
Data locality on Tensorflow.


Scenario -> actions:
* First time         -> Clone project + Build the image
* Usual work session -> Work with project


Actions:
* Clone project:
  * git clone https://github.com/saulam/daloflow.git
  * git submodule update --init --recursive
* Build the image:
  * docker image build -t daloflow:0.1 .
* Work with project:
  * docker run --network host -v $(pwd):/usr/src/daloflow -it daloflow:0.1 bash
  + <work session>
    * ./mpich-build.sh		
    * ./tensorflow-build.sh
    * ./horovod-build.sh	
  * exit


Unsorted actions:
* Run docker in docker:
  * docker run -v /var/run/docker.sock:/var/run/docker.sock <other options>
* Run with docker compose:
  * docker-compose -f Dockercompose.yml up -d
  * <work session>
  * docker-compose -f Dockercompose.yml down
* Modify Dockerfile:
  * vi Dockerfile
  * <edit the options needed>
  * docker image build -t daloflow:0.1 .
* Inspect:
  * docker inspect -f '{{range .NetworkSettings.Networks}}{{.IPAddress}}{{end}}' daloflow

