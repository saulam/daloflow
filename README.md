# daloflow
Data locality on Tensorflow.


Clone project (1 time)
* git clone https://github.com/saulam/daloflow.git
* git submodule update --init --recursive
* docker image build -t daloflow:0.1 .

Work with project (each time)
* docker run --network host -v $(pwd):/usr/src/daloflow -it daloflow:0.1 bash
+ <work session>
* exit

Work session (each time)
* ./sources-build.sh


Unsorted
* Run docker in docker:
  * docker run -v /var/run/docker.sock:/var/run/docker.sock <other options>
* Run With docker compose:
  * docker-compose -f Dockercompose.yml up -d daloflow 
  * <work session>
  * docker-compose -f Dockercompose.yml down
* Prepare docker image (1 time)
  * vi Dockerfile
  * <edit the options needed>
  * docker image build -t daloflow:0.1 .
* Inspect
  * docker inspect -f '{{range .NetworkSettings.Networks}}{{.IPAddress}}{{end}}' daloflow

