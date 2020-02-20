
ARG UBUNTU_VERSION=18.04
FROM ubuntu:${UBUNTU_VERSION} AS base


# Install initial software
RUN apt-get update && apt-get install -y --allow-downgrades --allow-change-held-packages --no-install-recommends \
        aptitude \
        build-essential \
        cmake \
        g++ \
        git \
        curl \
        vim \
        wget \
        ca-certificates \
        libjpeg-dev \
        libpng-dev \
        python3 \
        python3-dev \
        python3-pip \
        librdmacm1 \
        libibverbs1 \
        ibverbs-providers \
        libcurl3-dev \
        libfreetype6-dev \
        libhdf5-serial-dev \
        libzmq3-dev \
        pkg-config \
        rsync \
        apt-utils \
        software-properties-common \
        sudo \
        gpg-agent \
        unzip \
        zip \
        zlib1g-dev \
        virtualenv \
        swig \
        openjdk-8-jdk \
        openjdk-8-jre-headless \
        && \
    apt-get clean && \
    rm -rf /var/lib/apt/lists/*


# Install bazel
RUN curl https://bazel.build/bazel-release.pub.gpg | sudo apt-key add -
RUN echo "deb [arch=amd64] https://storage.googleapis.com/bazel-apt stable jdk1.8" | sudo tee /etc/apt/sources.list.d/bazel.list
RUN sudo apt-get update && sudo apt-get install bazel


# Install python software
RUN pip3 --no-cache-dir install \
    Pillow \
    h5py \
    wheel \
    typing \
    keras_preprocessing \
    matplotlib \
    mock \
    numpy \
    scipy \
#    sklearn \
    pandas \
    setuptools \
    mock \
#    future \
#    portpicker \
    enum34


# Install OpenSSH for MPI to communicate between containers
RUN apt-get install -y --no-install-recommends openssh-client openssh-server && \
    mkdir -p /var/run/sshd

# Allow OpenSSH to talk to containers without asking for confirmation
RUN cat /etc/ssh/ssh_config | grep -v StrictHostKeyChecking > /etc/ssh/ssh_config.new && \
    echo "    StrictHostKeyChecking no" >> /etc/ssh/ssh_config.new && \
    mv /etc/ssh/ssh_config.new /etc/ssh/ssh_config

# Download examples
RUN apt-get install -y --no-install-recommends subversion && \
    svn checkout https://github.com/horovod/horovod/trunk/examples && \
    rm -rf /examples/.svn

WORKDIR "/examples"
EXPOSE 22
CMD ["/usr/sbin/sshd", "-D"]
