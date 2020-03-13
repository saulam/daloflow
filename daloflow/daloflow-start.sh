#!/bin/bash
set -x


#
# Usage
#

if [ $# -lt 1 ]; then
	echo "Usage: $0 <number of node in the cluster>"
	exit
fi


#
# Start container cluster
#

docker-compose -f Dockercompose.yml up -d --scale node=$1


#
# Setup container cluster
#

CONTAINER_ID_LIST=$(docker ps|grep daloflow_node|cut -f1 -d' ')

docker inspect -f '{{range .NetworkSettings.Networks}}{{.IPAddress}}{{end}}' $CONTAINER_ID_LIST > machines_mpi
docker inspect -f '{{range .NetworkSettings.Networks}}{{.IPAddress}}{{end}}' $CONTAINER_ID_LIST | sed 's/.*/& slots=1/g' > machines_horovod

