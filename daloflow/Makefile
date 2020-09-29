.ONESHELL:
all: help


help:
	@echo ""
	@echo "  daloflow 1.0"
	@echo " --------------"
	@echo ""
	@echo ": Available options for first time deployment:"
	@echo "  make prerequisites"
	@echo "  make clone"
	@echo "  make image"
	@echo ""
	@echo ": Available options for session management:"
	@echo "  make start nc=<number of container>"
	@echo "  make status"
	@echo "  make stop"
	@echo ""
	@echo ": Available options in a typical work session:"
	@echo "  make build"
	@echo "  make test"
	@echo "  make mpirun np=2 a=\"python3 ./horovod/examples/tensorflow2_mnist.py\""
	@echo "  make bash c=<id container, from 1 up to nc>"
	@echo ""
	@echo ": Please read the README.md file for more information."
	@echo ""


prerequisites:
	@echo ""
	@echo "pre-requisites..."
	@echo ""

	# DOCKER
	sudo apt-get update
	sudo apt-get upgrade
	sudo apt-get install curl apt-transport-https ca-certificates software-properties-common
	curl -fsSL https://download.docker.com/linux/ubuntu/gpg | sudo apt-key add -
	sudo add-apt-repository "deb [arch=amd64] https://download.docker.com/linux/ubuntu $(lsb_release -cs) stable"
	sudo apt-get update
	sudo apt install docker-ce docker-ce-cli containerd.io

	# DOCKER-COMPOSER
	pip3 install docker-compose


clone:
	@echo ""
	@echo "clone daloflow and download mpich 332 and tensorflow 201..."
	@echo ""

	git clone https://github.com/saulam/daloflow.git
	cd daloflow
	git submodule update --init --recursive

	# MPI
	wget http://www.mpich.org/static/downloads/3.3.2/mpich-3.3.2.tar.gz
	rm -fr mpich
	tar zxf mpich-3.3.2.tar.gz
	mv mpich-3.3.2 mpich

	# TENSORFLOW
	wget https://github.com/tensorflow/tensorflow/archive/v2.0.1.tar.gz
	rm -fr tensorflow
	tar zxf v2.0.1.tar.gz
	mv tensorflow-2.0.1 tensorflow

	# HOROVOD
	wget https://github.com/horovod/horovod/archive/v0.19.0.tar.gz


image:
	@echo ""
	@echo "image..."
	@echo ""

	docker image build -t daloflow:1 .


build:
	@echo ""
	@echo "build..."
	@echo ""

	./daloflow/daloflow-build.sh


start:
	@echo ""
	@echo "start..."
	@echo ""

	./daloflow/daloflow-start.sh $(nc)


status:
	@echo ""
	@echo "status..."
	@echo ""

	# Status
	docker ps


mpirun:
	@echo ""
	@echo "mpirun..."
	@echo ""

	# Please execute:
	@echo ""
	docker container exec -it daloflow_node_1 mpirun -np $(np) -machinefile machines_mpi -bind-to none -map-by slot $(a)
	@echo ""


stop:
	@echo ""
	@echo "stop..."
	@echo ""

	# Stop container cluster
	docker-compose -f Dockercompose.yml down


test:
	@echo ""
	@echo "test..."
	@echo ""

	#
	# MPICH
	#
	docker container exec -it daloflow_node_1 ./daloflow/daloflow-test.sh


bash:
	@echo ""
	@echo "bash..."
	@echo ""

	#
	# bash
	#
	docker container exec -it daloflow_node_$(c) /bin/bash


