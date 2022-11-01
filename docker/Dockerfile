FROM ubuntu:20.04 as builder

ENV DEBIAN_FRONTEND=noninteractive

COPY --from=translate5/proxygen:latest / /

RUN apt update && apt install -y git libxerces-c-dev libicu-dev libc-dev gcc libglib2.0-dev libffi-dev build-essential  \
    libsodium-dev libzstd-dev gdb rsync zip openssh-server cmake libboost-all-dev libdouble-conversion-dev libssl-dev libgoogle-glog-dev gperf -y

RUN cd /home/ &&  git clone https://github.com/translate5/translate5-tm-service-source.git
RUN cd /home/translate5-tm-service-source && git checkout changing_http_lib_proxygen

#add sodium
#setup libs to use with t5memory
RUN mkdir -p /opt/local/lib/ && cp /usr/lib/x86_64-linux-gnu/libsodium.a /opt/local/lib/libsodium.a
RUN mkdir -p /home/libs/proxygen/_build/deps/lib/ && cp -R /home/translate5-tm-service-source/docker/lib/* /home/libs/proxygen/_build/deps/lib/

RUN cd /home/translate5-tm-service-source && ./build.sh


FROM ubuntu:20.04

ENV DEBIAN_FRONTEND=noninteractive

RUN apt update && apt install -y libgflags-dev libssl-dev libxerces-c-dev libsodium-dev libzstd-dev  \
    libgoogle-glog-dev libdouble-conversion-dev libboost-all-dev

COPY --from=builder /home/translate5-tm-service-source/_build/t5memory_DEBUG  /root/t5memory_DEBUG
RUN mkdir /root/.t5memory
COPY --from=builder /home/translate5-tm-service-source/docker/workdir/* /root/.t5memory/

WORKDIR /root

ENTRYPOINT [ "/root/t5memory_DEBUG", "--port=8080", "--alsologtostderr=1", "--v=0" ]
EXPOSE 8080
