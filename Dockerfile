
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
        python-setuptools \
        zlib1g-dev \
        virtualenv \
        autoconf \
        libtool \
        swig \
        openjdk-8-jdk \
        openjdk-8-jre-headless \
        net-tools \
        && \
    apt-get clean && \
    rm -rf /var/lib/apt/lists/*


# Install bazel
RUN curl https://bazel.build/bazel-release.pub.gpg | sudo apt-key add -
RUN echo "deb [arch=amd64] https://storage.googleapis.com/bazel-apt stable jdk1.8" | sudo tee /etc/apt/sources.list.d/bazel.list
RUN sudo apt-get update && sudo apt-get install bazel && apt-get install -y bazel-1.2.1

# (begin) install bazel 0.26.1 for tensorflow 2.0.1
RUN wget https://github.com/bazelbuild/bazel/releases/download/0.26.1/bazel-0.26.1-linux-x86_64
RUN chmod a+x bazel-0.26.1-linux-x86_64
RUN bazel-0.26.1-linux-x86_64 /usr/bin/bazel
# (end)   install bazel 0.26.1 for tensorflow 2.0.1


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
    pandas \
    setuptools \
    mock \
    enum34

RUN pip3 --no-cache-dir install \
    sklearn \
    future \
    portpicker


# OpenSSH: Install for MPI to communicate between containers
RUN apt-get install -y --no-install-recommends openssh-client openssh-server
RUN mkdir -p /var/run/sshd

# OpenSSH: Allow Root login
RUN sed 's@session\s*required\s*pam_loginuid.so@session optional pam_loginuid.so@g' -i /etc/pam.d/sshd
RUN sed -i 's/PermitRootLogin prohibit-password/#PermitRootLogin prohibit-password/' /etc/ssh/sshd_config
RUN echo "PermitRootLogin yes" >> /etc/ssh/sshd_config
# OpenSSH: Allow to talk to containers without asking for confirmation
RUN cat /etc/ssh/ssh_config | grep -v StrictHostKeyChecking > /etc/ssh/ssh_config.new && \
    echo "    StrictHostKeyChecking no" >> /etc/ssh/ssh_config.new && \
    mv /etc/ssh/ssh_config.new /etc/ssh/ssh_config
# OpenSSH: keygen
RUN ssh-keygen -q -t rsa -N "" -f /root/.ssh/id_rsa
RUN cp /root/.ssh/id_rsa.pub /root/.ssh/authorized_keys

# Initial env
RUN echo 'root:daloflow' | chpasswd
WORKDIR "/usr/src/daloflow"

EXPOSE 22
CMD ["/usr/sbin/sshd", "-D"]

