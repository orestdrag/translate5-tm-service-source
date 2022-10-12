FROM sptrakesh/proxygen

RUN apk update
RUN apk search -v

RUN apk add xerces-c-dev icu-dev libc-dev gcc glib-dev libffi-dev build-base libressl-dev libsodium-dev zstd-dev gdb  rsync zip \
	openssh openssh-server openrc \
	#glog-dev \
	libexecinfo-dev \
    libexecinfo

RUN apk add --upgrade glog-dev

RUN git clone https://github.com/translate5/translate5-tm-service-source.git /home
#RUN cd /home && git checkout 1b1ce5ba4d1aa92c7a5d6ea2c47f44c251916e0c
RUN cd /home && git checkout changing_http_lib_proxygen

RUN cp -r /home/docker/lib/cmake/sodium /opt/local/lib/cmake/sodium
RUN cp -r /home/docker/workdir /root/.t5memory
#COPY workdir/t5memory.conf /root/.t5memory/

RUN cd /home && /home/build.sh
RUN cp /home/_build/t5memory_DEBUG  /root/t5memory_DEBUG

EXPOSE 4040 8080

WORKDIR /root

ENTRYPOINT [ "/root/t5memory_DEBUG", "---port=8080", "---alsologtostderr=1", "--v=0" ]
