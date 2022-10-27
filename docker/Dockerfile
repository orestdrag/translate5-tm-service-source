FROM ubuntu:20.04 as build

ENV DEBIAN_FRONTEND=noninteractive

RUN apt update && apt install -y git libxerces-c-dev libicu-dev libc-dev gcc libglib2.0-dev libffi-dev build-essential libsodium-dev libzstd-dev gdb rsync zip openssh-server cmake libboost-all-dev libdouble-conversion-dev libssl-dev libgoogle-glog-dev gperf -y

RUN mkdir /home/libs && git clone https://github.com/facebook/proxygen.git /home/libs

#next line - fixed fizz build after commits in fizz-main 18 Oct 2022 
RUN echo "Subproject commit 7401af7ee2db6d014730da9c46643926f0901d02" > /home/libs/build/deps/github_hashes/facebookincubator/fizz-rev.txt
RUN cd /home/libs/proxygen/ && ./build.sh --no-tests -j 1 && ./install.sh

#setup libs to use with t5memory
RUN mkdir -p /opt/local/lib/ && cp /usr/lib/x86_64-linux-gnu/libsodium.a /opt/local/lib/libsodium.a 

#cleanup repo 
RUN cd /home/libs/ && find . -type f \! \( -name '*.h' -or -name '*.cmake' -or -name '*.hpp' -or -name '*.txt' -or -name 'proxygen_*' -or  -name '*.a' -or -name '*so' -or -name '*.so*' \) -delete

#RUN cp -R /home/libs/proxygen/_build/* /opt/local/ && cp -R /home/libs/proxygen/_build/deps/* /opt/local/ 

#cleanup 
#RUN cd /opt/local/ && find . -type f \! \( -name '*.a' -or -name '*.cmake' -or -name 'proxygen_*' -or -name '*.h' #\) -delete
#RUN rm -R deps proxygen/lib fizz/build fmt/build generated/ googletest/build wangle/build share zstd/build 

#RUN cp /usr/lib/x86_64-linux-gnu/libsodium.a /home/libs/proxygen/_build/deps/lib/libsodium.a 

#cleanup 
# RUN rm -R /opt/local/deps && rm -R /home/libs 
#RUN cd /home/libs/proxygen/_build/deps && find .  -type f \! \( -wholename './lib*' -or -wholename './bin*'  -or -wholename '*include*'  \)  -delete 

RUN cd /home/libs/proxygen/_build && find .  -type f \! \( -wholename './lib*' -or -wholename './bin*'  -or -wholename '*include*' -or -wholename './deps/bin*' -or -wholename './deps/lib*'  \)  -delete


#RUN rm -R fmt/.git fmt/.github fmt/test fmt/doc fmt/scr fmt/support fmt/build 
#RUN rm -R fizz/build 
#RUN rm -R generated share  
#RUN rm -R googletest/build wangle/build zstd/build

# clone t5memory repo
RUN cd /home/ &&  git clone https://github.com/translate5/translate5-tm-service-source.git 
RUN cd /home/translate5-tm-service-source && git checkout changing_http_lib_proxygen

#add sodium
RUN cp -R /home/translate5-tm-service-source/docker/lib/* /home/libs/proxygen/_build/deps/lib/

