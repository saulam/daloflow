#!/bin/bash
set -x

#
# Get configuration
#

BASE_DIR=$(dirname "$0")
. $BASE_DIR/config.copy

if [ ! -d "$BASE_CACHE" ]; then
        echo "Directory not found: $BASE_CACHE"
        exit
fi

if [ ! -f "$LIST_CACHE" ]; then
        echo "File not found: $LIST_CACHE"
        exit
fi


#
# Copy files into a local directory ($BASE_CACHE)
#

# Remove old "train*.tar.gz" files at /mnt/local-storage/daloflow/dataset/
find $BASE_CACHE -name "train*.tar.gz" -exec rm  {} \;

# Copy new files
$BASE_DIR/hdfs-cp.sh $LIST_CACHE $BASE_CACHE

