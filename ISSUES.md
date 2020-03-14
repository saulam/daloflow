# daloflow
Data locality on Tensorflow.


ISSUES:
* make build que haga el docker exec.... "make build" actual

Unsorted actions:
* Run docker in docker:
  * docker run -v /var/run/docker.sock:/var/run/docker.sock <other options>
* Clean all images (warning):
  * docker rmi -f $(docker images -q)
* Change daloflow path:
  * find ./ -type f -exec sed -i 's/\/usr\/src\/daloflow/yourPath/g' {} \;
* Compile project:
  * docker run --network host -v $(pwd):/usr/src/daloflow -it daloflow:1 bash
  * ./mpich-build.sh		
  * ./tensorflow-build.sh
  * ./horovod-build.sh	
  * exit

