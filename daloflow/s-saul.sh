rm -fr Dockercompose.yml
rm -fr Dockerfile
rm -fr Dockerstack.yml
ln -s docker/Dockercompose.yml-salonso Dockercompose.yml 
ln -s docker/Dockerfile-gpu-daloflow    Dockerfile
ln -s docker/Dockerstack.yml-salonso   Dockerstack.yml 
