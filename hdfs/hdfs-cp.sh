#!/bin/bash
#set -x

# For HADOOP (José)
export PATH=/mnt/local-storage/prueba-hdfs/hadoop/bin:/usr/lib/jvm/java-8-openjdk-amd64/bin:$PATH
export JAVA_HOME=/usr/lib/jvm/java-8-openjdk-amd64/jre
export HADOOP_INSTALL=/mnt/local-storage/prueba-hdfs/hadoop
export PATH=$PATH:/mnt/local-storage/prueba-hdfs/hadoop/bin:/mnt/local-storage/prueba-hdfs/hadoop/sbin
export HADOOP_MAPRED_HOME=$HADOOP_INSTALL
export HADOOP_COMMON_HOME=$HADOOP_INSTALL
export HADOOP_HDFS_HOME=$HADOOP_INSTALL
export YARN_HOME=$HADOOP_INSTALL
export HADOOP_COMMON_LIB_NATIVE=$HADOOP_INSTALL/lib/native
export HADOOP_OPTS="-Djava.library.path=$HADOOP_INSTALL/lib"
export HADOOP_PORT=0

# LD_LIBRARY_PATH
FULL_LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/home/jrivadeneira/lib-hdfs/lib:/usr/lib/jvm/java-8-openjdk-amd64/jre/lib/amd64/server

# CLASSPATH
FULL_CLASSPATH=$CLASSPATH:/home/jrivadeneira/Documentos/jar-hdfs
FULL_CLASSPATH=$FULL_CLASSPATH:$(hadoop classpath --glob)


# bind everything all together
env CLASSPATH=$FULL_CLASSPATH LD_LIBRARY_PATH=$FULL_LD_LIBRARY_PATH ./hdfs-cp $@

