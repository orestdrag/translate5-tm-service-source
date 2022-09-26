FROM sptrakesh/proxygen  as T5MEMORY

#ENV DEBIAN_FRONTEND=noninteractive

RUN apk update
RUN apk search -v

RUN apk add xerces-c-dev icu-dev libc-dev gcc glib-dev libffi-dev build-base libressl-dev libsodium-dev zstd-dev gdb  rsync zip openssh openssh-server openrc #boost-dev musl-dev  clang

#RUN cd /home && git clone https://github.com/translate5/translate5-tm-service-source.git source
#RUN cd /home/source && git checkout changing_http_lib_proxygen
COPY source /home/source
COPY mri /home/mri
COPY include /home/include
COPY .git /home/.git
COPY build.sh /home/build.sh

COPY docker/lib /opt/local/lib
COPY docker/workdir /root/.t5memory

RUN cd /home && ./build.sh
RUN cp /home/_build/t5memory_DEBUG  /root/t5memory_DEBUG
#RUN rm -R /home/*

ENTRYPOINT /root/t5memory_DEBUG


# configure SSH for communication with Visual Studio 
RUN mkdir -p /var/run/sshd
RUN echo 'root:root' | chpasswd \
    && sed -i 's/#PermitRootLogin prohibit-password/PermitRootLogin yes/' /etc/ssh/sshd_config #\
   # && sed 's@session\s*required\s*pam_loginuid.so@session optional pam_loginuid.so@g' -i /etc/pam.d/sshd
s

RUN sed -i 's/#PasswordAuthentication yes/PasswordAuthentication yes/g' /etc/ssh/sshd_config
RUN sed -i 's/#PubkeyAuthentication yes/PubkeyAuthentication no/g' /etc/ssh/sshd_config
#RUN service sshd start
#RUN rc=update add sshd

CMD ["/usr/sbin/sshd", "-D"]

EXPOSE 4040 2222 22 4044

