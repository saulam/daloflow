#!/bin/bash
#set -x

# Load default configuration
BASE_DIR=$(dirname "$0")
. $BASE_DIR/config.hdfs-cp

# For HADOOP (José)
export PATH=$BASE_HDFS/bin:$BASE_JAVA/bin::$BASE_JAVA/sbin:$PATH
export JAVA_HOME=$BASE_JAVA/jre
export HADOOP_INSTALL=$BASE_HDFS
export HADOOP_MAPRED_HOME=$BASE_HDFS
export HADOOP_COMMON_HOME=$BASE_HDFS
export HADOOP_HDFS_HOME=$BASE_HDFS
export YARN_HOME=$BASE_HDFS
export HADOOP_COMMON_LIB_NATIVE=$BASE_HDFS/lib/native
export HADOOP_OPTS="-Djava.library.path=$BASE_HDFS/lib"
export HADOOP_PORT=0

# LD_LIBRARY_PATH + CLASSPATH
FULL_LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$BASE_LIBHDFS/lib:$BASE_JAVA/jre/lib/amd64/server
FULL_CLASSPATH=$CLASSPATH:$(hadoop classpath --glob)


# bind everything all together
env CLASSPATH=$FULL_CLASSPATH LD_LIBRARY_PATH=$FULL_LD_LIBRARY_PATH $BASE_DIR/hdfs-cp $@

