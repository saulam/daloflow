#!/bin/bash
set -x

#
# Config.
#
BASE_CACHE=/mnt/local-storage/daloflow/dataset/
LIST_CACHE=$BASE_CACHE/list.txt


#
# Cache files
#

if [ ! -d "$BASE_CACHE" ]; then
	echo "Directory not found: $BASE_CACHE"
	exit
fi

if [ ! -f "$LIST_CACHE" ]; then
	echo "File not found: $LIST_CACHE"
	exit
fi


# remove old "train*.tar.gz" files at /mnt/local-storage/daloflow/dataset/
find $BASE_CACHE -name "train*.tar.gz" -exec rm  {} \;

# copy new files
./hdfs-cp.sh $LIST_CACHE $BASE_CACHE

