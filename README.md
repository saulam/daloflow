# daloflow
Data locality on Tensorflow.


[Prepare docker in docker] (0 or 1 time)
* docker run -v /var/run/docker.sock:/var/run/docker.sock <other options>

Clone project (1 time)
* git clone https://github.com/saulam/daloflow.git
* git submodule update --init --recursive

Prepare docker image (1 time)
* vi Dockerfile
  <edit the options needed>
* docker image build -t daloflow:0.1 .

Run docker (each time)
* With docker compose:
  * docker-compose -f Dockercompose.yml up -d daloflow 
  * docker inspect -f '{{range .NetworkSettings.Networks}}{{.IPAddress}}{{end}}' daloflow
    <work session>
  * docker-compose -f Dockercompose.yml down
* With docker cli:
  * docker run -v $(pwd):/usr/src/daloflow -it bash daloflow:0.1
    <work session>
  * exit

Within docker container (each time)
* ./sources-build.sh

