#!/bin/bash
set -x


#
# Install each node
#

CONTAINER_ID_LIST=$(docker ps|grep daloflow_node|cut -f1 -d' ')

for C in $CONTAINER_ID_LIST; do
	docker container exec -it $C ./daloflow/daloflow-build-node.sh ;
done

