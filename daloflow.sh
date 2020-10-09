#!/bin/bash
set -x


daloflow_help ()
{
	echo ""
	echo "  daloflow 3.0"
	echo " --------------"
	echo ""
	echo ": For first time deployment, please execute:"
	echo "  $0 postclone"
	echo "  $0 prerequisites"
	echo "  $0 image"
	echo ""
	echo ": For a typical single node work session, please execute:"
	echo "  $0 start <number of container>"
	echo "  $0 mpirun <np> \"python3 ./horovod/examples/tensorflow2_keras_mnist.py\""
	echo "  ..."
	echo "  $0 stop"
	echo ""
	echo ": For a typical multinode work session, please execute:"
	echo "  $0 swarm-start <number of container>"
	echo "  $0 swarm-mpirun <np> \"python3 ./horovod/examples/tensorflow2_keras_mnist.py\""
	echo "  ..."
	echo "  $0 swarm-stop"
	echo ""
	echo ": Available options for (single node) debugging:"
	echo "  $0 status"
	echo "  $0 test"
	echo "  $0 bash <id container, from 1 up to nc>"
	echo "  $0 save"
	echo "  $0 load"
	echo ""
	echo ": Please read the README.md file for more information."
	echo ""
}


daloflow_postclone ()
{
	echo "Downloading mpich 3.3.2, tensorflow 2.0.1, and Horovod 0.19.0..."

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
	tar zxf v0.19.0.tar.gz
	mv horovod-0.19.0 horovod
}

daloflow_prerequisites ()
{
	echo "Installing Docker and Docker-compose..."

	# To Install DOCKER
	sudo apt-get update
	sudo apt-get upgrade
	sudo apt-get install -y curl apt-transport-https ca-certificates software-properties-common
	curl -fsSL https://download.docker.com/linux/ubuntu/gpg | sudo apt-key add -
        curl https://bazel.build/bazel-release.pub.gpg | sudo apt-key add -
	sudo apt-get update
	sudo add-apt-repository "deb [arch=amd64] https://download.docker.com/linux/ubuntu $(lsb_release -cs) stable"
	sudo apt-get update
	sudo apt install -y docker-ce docker-ce-cli containerd.io

	# To Install DOCKER-COMPOSER
	pip3 install docker-compose

	# NVIDIA GPU: https://nvidia.github.io/nvidia-container-runtime/
	curl -s -L https://nvidia.github.io/nvidia-container-runtime/gpgkey | sudo apt-key add -
	distribution=$(. /etc/os-release;echo $ID$VERSION_ID)
	curl -s -L https://nvidia.github.io/nvidia-container-runtime/$distribution/nvidia-container-runtime.list | \
		  sudo tee /etc/apt/sources.list.d/nvidia-container-runtime.list
	sudo apt-get update
	apt-get install -y nvidia-container-runtime
        docker run -it --rm --gpus all ubuntu nvidia-smi
}

daloflow_image ()
{
	echo "Building initial image..."
	docker image build -t daloflow:v1 .

	echo "BASE_HOME=$(pwd)" > .env

	echo "Building compilation image..."
	daloflow_start 1
        daloflow_build
	CONTAINER_ID_LIST=$(docker ps|grep daloflow_node|cut -f1 -d' ')
	docker commit $CONTAINER_ID_LIST daloflow:latest
	docker-compose -f Dockercompose.yml down
}

daloflow_build_node ()
{
	# MPICH
	# from source # ./mpich-build.sh
	./mpich-build.sh

	# TENSORFLOW
        # from source # ./tensorflow-build.sh
        # from wheel  # pip3 install ./daloflow/tensorflow-2.0.1-cp36-cp36m-linux_x86_64.whl
        pip3 install ./daloflow/tensorflow-2.0.1-cp36-cp36m-linux_x86_64.whl

	# HOROVOD
	# from source #./horovod-build.sh
	# from wheel  # HOROVOD_WITH_MPI=1 HOROVOD_WITH_TENSORFLOW=1 pip3 install ./daloflow/horovod-0.19.0-cp36-cp36m-linux_x86_64.whl
	HOROVOD_WITH_MPI=1 HOROVOD_WITH_TENSORFLOW=1 pip3 install --no-cache-dir horovod
}

daloflow_build ()
{
	# Install each node
	CONTAINER_ID_LIST=$(docker ps|grep daloflow_node|cut -f1 -d' ')
	for C in $CONTAINER_ID_LIST; do
		docker container exec -it $C ./daloflow.sh build_node 
	done
}

daloflow_start ()
{
	# Setup number of containers
	NC=$1
	if [ $# -lt 1 ]; then
	     NC=1
	fi

	# Start container cluster
	docker-compose -f Dockercompose.yml up -d --scale node=$NC
	echo "wating $NC seconds..."
	sleep $NC

	# Setup container cluster
	CONTAINER_ID_LIST=$(docker ps -f name=daloflow -q)
	docker inspect -f '{{range .NetworkSettings.Networks}}{{.IPAddress}}{{end}}' $CONTAINER_ID_LIST > machines_mpi
	docker inspect -f '{{range .NetworkSettings.Networks}}{{.IPAddress}}{{end}}' $CONTAINER_ID_LIST | sed 's/.*/& slots=1/g' > machines_horovod
}

daloflow_test ()
{
	# MPICH
	# execute cpi
	cd /usr/src/daloflow/mpich/examples
	mpicc -o cpi cpi.c
	mpirun -np 2 -machinefile /usr/src/daloflow/machines_mpi $(pwd)/cpi

	# HOROVOD
	mpirun -np 2 -machinefile machines_mpi -bind-to none -map-by slot python3 ./horovod/examples/tensorflow2_mnist.py
	# horovodrun --verbose -np 2 -hostfile machines_horovod  python3 ./horovod/examples/tensorflow2_mnist.py
}

daloflow_swarm_start ()
{
        # Setup number of containers
        NC=$1
        if [ $# -lt 1 ]; then
             NC=1
        fi

        # Start container cluster
        docker stack deploy --compose-file Dockerstack.yml daloflow
        docker service scale daloflow_node=$NC

        # Setup container cluster
        CONTAINER_ID_LIST=$(docker service ps daloflow_node -f desired-state=running -q)
        docker inspect -f '{{range .NetworksAttachments}}{{.Addresses}}{{end}}' $CONTAINER_ID_LIST | sed "s/^\[//g" | awk 'BEGIN {FS="/"} ; {print $1}' > machines_mpi
        cat machines_mpi | sed 's/.*/& slots=1/g' > machines_horovod
}


#
# Main
#

# Usage
if [ $# -eq 0 ]; then
	daloflow_help $0
	exit
fi

# for each argument, try to execute it
while (( "$#" )); 
do
	case $1 in
	     # first execution
	     prerequisites)
		daloflow_prerequisites
	     ;;
	     postclone)
		daloflow_postclone
	     ;;

	     # image
	     image)
		daloflow_image
	     ;;
	     build)
		daloflow_build
	     ;;
	     build_node)
		daloflow_build_node
	     ;;
	     save)
		echo "Saving image..."
	        IMAGE_ID_LIST=$(docker image ls|grep daloflow|grep latest|awk '{print $3}')
		docker image save daloflow:latest | gzip -5 > daloflow_v2.tgz 
	     ;;
	     load)
		echo "Loading image..."
		cat daloflow_v2.tgz | gunzip - | docker image load
	     ;;

	     # single node
	     start)
		shift
		daloflow_start $1
	     ;;
	     mpirun)
		shift
		NP=$1
		shift
		A=$1
		CNAME="daloflow_node_1"
		docker container exec -it $CNAME mpirun -np $NP -machinefile machines_mpi -bind-to none -map-by slot $A
	     ;;
	     stop)
		docker-compose -f Dockercompose.yml down
	        rm -fr machines_mpi
	        rm -fr machines_horovod
	     ;;

	     # multinode node
	     swarm-start)
		shift
		daloflow_swarm_start $1
	     ;;
	     swarm-mpirun)
		shift
		NP=$1
		shift
		A=$1
		CNAME=$(docker ps -f name=daloflow -q | head -1)
		docker container exec -it $CNAME mpirun -np $NP -machinefile machines_mpi -bind-to none -map-by slot $A
	     ;;
	     swarm-stop)
                docker service rm daloflow_node
	        rm -fr machines_mpi
	        rm -fr machines_horovod
	     ;;

	     # single node utilities
	     status|ps)
		docker ps
	     ;;
	     bash)
		shift
		CIP=$(head -$1 machines_mpi | tail -1)
		docker container exec -it $(docker ps -f name=daloflow -q | head -1) /usr/bin/ssh $CIP
	     ;;
	     test)
		docker container exec -it daloflow_node_1 ./daloflow.sh test_node
	     ;;
	     test_node)
		daloflow_test
	     ;;

	     # help
	     *)
		daloflow_help $0
	     ;;
	esac
	shift
done


