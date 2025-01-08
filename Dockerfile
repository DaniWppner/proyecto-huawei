FROM ubuntu:20.04

# Install dependencies
RUN apt-get -qq update; \
    apt-get install -qqy --no-install-recommends \
    wget xz-utils

# Install clang + llvm version 9.0.0
RUN mkdir /usr/local/clang+llvm
WORKDIR /usr/local/clang+llvm
# --no-check-certificate is necessary due to VPN shenanigans, should remove or fix otherwise in a better environment
RUN wget --no-verbose --show-progress --progress=bar:force:noscroll --no-check-certificate \
    https://releases.llvm.org/9.0.0/clang+llvm-9.0.0-x86_64-pc-linux-gnu.tar.xz 
RUN tar -xvf clang+llvm-9.0.0-x86_64-pc-linux-gnu.tar.xz
RUN rm clang+llvm-9.0.0-x86_64-pc-linux-gnu.tar.xz
ENV PATH="$PATH:/usr/local/clang+llvm"

