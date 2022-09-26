FROM sptrakesh/proxygen  as T5MEMORY

ENV DEBIAN_FRONTEND=noninteractive

RUN apk update
RUN apk search -v

RUN apk add xerces-c-dev icu-dev libc-dev gcc glib-dev libffi-dev build-base libressl-dev libsodium-dev zstd-dev gdb  rsync zip openssh-server

RUN cd /home && git clone https://github.com/translate5/translate5-tm-service-source.git source
RUN cd /home/source && git checkout changing_http_lib_proxygen

RUN cd /home/source/source/build && ./build.sh
RUN mkdir /home/build
RUN mkdir /home/newbuild
RUN cp /home/source/source/build/t5memory_DEBUG /home/build/t5memory

COPY workdir /root/.t5memory

#RUN rm -R /home/source

ENTRYPOINT /home/build/t5memory

EXPOSE 4040 2222

