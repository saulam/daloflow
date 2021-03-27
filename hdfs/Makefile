
#
# Const.
#

PRUEBA_HDFS=/mnt/local-storage/prueba-hdfs/hadoop/
CFLAGS=-Wall -g
LDFLAGS=-lhdfs -lpthread -ljvm
LIB_PATH=-L/usr/lib/jvm/java-8-openjdk-amd64/jre/lib/amd64/server -L$(PRUEBA_HDFS)/lib/native
INCLUDE_PATH=-I$(PRUEBA_HDFS)/include


#
# Rules
#

#all:: hdfs-cp hdfs-test
all:: hdfs-cp

clean::
	rm -fr hdfs-cp hdfs-test

hdfs-cp:
	gcc $(CFLAGS) $(INCLUDE_PATH) $(LIB_PATH) hdfs-cp.c -o hdfs-cp $(LDFLAGS)

hdfs-test:
	gcc $(CFLAGS) $(INCLUDE_PATH) $(LIB_PATH) hdfs-test.c -o hdfs-test $(LDFLAGS)

