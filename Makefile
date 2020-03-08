all: help


help:
	@echo ""
	@echo "daloflow 0.5"
	@echo "------------"
	@echo ""
	@echo ": first time"
	@echo "make prerequisites"
	@echo "make clone"
	@echo "make image"
	@echo ""
	@echo ": work session"
	@echo "make start NC=<number of nodes>"
	@echo "make build"
	@echo "make stop"
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
	@echo "clone..."
	@echo ""

	git clone https://github.com/saulam/daloflow.git
	git submodule update --init --recursive


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
	cd /usr/src/daloflow/
	tar zxf mpich-3.3.2.tar.gz
	cd /usr/src/daloflow/mpich-3.3.2
	#
	./configure --enable-orterun-prefix-by-default --disable-fortran
	make -j $(nproc) all
	make install
	ldconfig 

	# TENSORFLOW
	cd /usr/src/daloflow/tensorflow
	# ./configure
	export PYTHON_BIN_PATH=`which python3`
	yes "" | $PYTHON_BIN_PATH configure.py
	# build
	bazel build --config=opt //tensorflow/tools/pip_package:build_pip_package --action_env PYTHON_BIN_PATH=/usr/bin/python3 
	./bazel-bin/tensorflow/tools/pip_package/build_pip_package /usr/src/daloflow/tensorflow/tensorflow_pkg
	pip3 install /usr/src/daloflow/tensorflow/tensorflow_pkg/tensorflow-*.whl

	# HOROVOD
	cd /usr/src/daloflow/horovod
	#
	python3 setup.py clean
	CFLAGS="-march=native -mavx -mavx2 -mfma -mfpmath=sse" python3 setup.py bdist_wheel
	pip3 install ./dist/horovod-*.whl


install:
	@echo ""
	@echo "install..."
	@echo ""

	# MPICH
	cd /usr/src/daloflow/mpich-3.3.2
	make install
	ldconfig 

	# TENSORFLOW
	cd /usr/src/daloflow/tensorflow
	pip3 install /usr/src/daloflow/tensorflow/tensorflow_pkg/tensorflow-*.whl

	# HOROVOD
	cd /usr/src/daloflow/horovod
	pip3 install ./dist/horovod-*.whl


start:
	@echo ""
	@echo "start..."
	@echo ""

	# Check NP
ifeq ("$(NC)", "")
	@echo "Usage: make start NC=<number of node in the cluster>"
	exit
else
	# Start container cluster
	docker-compose -f Dockercompose.yml up -d --scale node=$(NC)

	# Setup container cluster
	CONTAINER_ID_LIST=$(docker ps|grep daloflow_node|cut -f1 -d' ')
	for C in $CONTAINER_ID_LIST; do 
		docker container exec -it $C ./daloflow-install.sh ; 
	done

	docker inspect -f '{{range .NetworkSettings.Networks}}{{.IPAddress}}{{end}}' $CONTAINER_ID_LIST > machines_mpi
	docker inspect -f '{{range .NetworkSettings.Networks}}{{.IPAddress}}{{end}}' $CONTAINER_ID_LIST | sed 's/.*/& slots=1/g' > machines_horovod
endif


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
	cd /usr/src/daloflow/mpich/examples
	mpicc -o cpi cpi.c
	mpirun -np 2 -machinefile /usr/src/daloflow/machines_mpi $(pwd)/cpi
	#
	# HOROVOD
	#
	mpirun -np 2 -machinefile machines_mpi -bind-to none -map-by slot python3 ./horovod/examples/tensorflow2_mnist.py
	# horovodrun --verbose -np 2 -hostfile machines_horovod  python3 ./horovod/examples/tensorflow2_mnist.py

