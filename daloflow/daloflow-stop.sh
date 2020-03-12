#!/bin/bash
set -x


#
# Usage
#

if [ $# -ne 0 ]; then
	echo "Usage: $0"
	exit
fi


#
# Stop container cluster
#

docker-compose -f Dockercompose.yml down

