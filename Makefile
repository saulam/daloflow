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
	@echo ": Available options for a work session:"
	@echo "  make start nc=<number of nodes>"
	@echo "  make status"
	@echo "  make stop"
	@echo ""
	@echo ": Please read the README.md file for more information."
	@echo "  make build"
	@echo "  make test"
	@echo "  make mpirun a=\"python3 ./horovod/examples/tensorflow2_mnist.py\""
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

	# MPICH
	cd /usr/src/daloflow/mpich && \
	./configure --enable-orterun-prefix-by-default --disable-fortran && \
	make -j $(nproc) all && \
	make install && \
	ldconfig 

	# TENSORFLOW
	cd /usr/src/daloflow/tensorflow && \
	export PYTHON_BIN_PATH=`which python3` && \
	yes "" | $(which python3) configure.py && \
	bazel build --config=opt //tensorflow/tools/pip_package:build_pip_package --action_env PYTHON_BIN_PATH=/usr/bin/python3 && \
	./bazel-bin/tensorflow/tools/pip_package/build_pip_package /usr/src/daloflow/tensorflow/tensorflow_pkg && \
	pip3 install /usr/src/daloflow/tensorflow/tensorflow_pkg/tensorflow-*.whl

	# HOROVOD
	cd /usr/src/daloflow/horovod && \
	python3 setup.py clean && \
	CFLAGS="-march=native -mavx -mavx2 -mfma -mfpmath=sse" python3 setup.py bdist_wheel && \
	pip3 install ./dist/horovod-*.whl


install:
	@echo ""
	@echo "install..."
	@echo ""

	# MPICH
	cd /usr/src/daloflow/mpich && \
	make install && ldconfig

	# TENSORFLOW
	cd /usr/src/daloflow/tensorflow && \
	pip3 install /usr/src/daloflow/tensorflow/tensorflow_pkg/tensorflow-*.whl

	# HOROVOD
	cd /usr/src/daloflow/horovod && \
	pip3 install ./dist/horovod-*.whl


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
	@echo "docker container exec -it daloflow_node_1 mpirun -np 2 -machinefile machines_mpi -bind-to none -map-by slot $(a)"
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
	cd /usr/src/daloflow/mpich/examples ; mpicc -o cpi cpi.c
	mpirun -np 2 -machinefile /usr/src/daloflow/machines_mpi /usr/src/daloflow/mpich/examples/cpi

	#
	# HOROVOD
	#
	mpirun -np 2 -machinefile machines_mpi -bind-to none -map-by slot python3 /usr/src/daloflow/horovod/examples/tensorflow2_mnist.py

