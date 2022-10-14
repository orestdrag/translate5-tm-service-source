FROM sptrakesh/proxygen  as T5MEMORY

RUN apk update
RUN apk search -v

RUN apk add xerces-c-dev icu-dev libc-dev gcc glib-dev libffi-dev \
		build-base libressl-dev libsodium-dev zstd-dev gdb \ 
		rsync zip openssh openssh-server openrc \
		libexecinfo-dev libexecinfo  zlib-dev

COPY source /home/source
COPY mri /home/mri
COPY include /home/include
COPY .git /home/.git
COPY build.sh /home/build.sh

COPY docker/lib /opt/local/lib
COPY docker/workdir /root/.t5memory

RUN cd /home && /home/build.sh
RUN cp /home/_build/t5memory_DEBUG  /root/t5memory_DEBUG

ENTRYPOINT /root/t5memory_DEBUG

EXPOSE 4040 2222 22 4044 4000

