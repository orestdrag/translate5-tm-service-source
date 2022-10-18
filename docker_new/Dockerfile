FROM ubuntu:20.04

ENV DEBIAN_FRONTEND=noninteractive

RUN apt update && apt install -y git libxerces-c-dev libicu-dev libc-dev gcc libglib2.0-dev libffi-dev build-essential libsodium-dev libzstd-dev gdb rsync zip openssh-server cmake libboost-all-dev libdouble-conversion-dev libssl-dev libgoogle-glog-dev gperf -y

RUN git clone https://github.com/facebook/proxygen.git /home

RUN cd /home/proxygen/ && ./build.sh 
#&& ./install.sh


