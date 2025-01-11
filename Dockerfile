FROM ubuntu:20.04

# Install dependencies
RUN apt-get -qq update; \
    apt-get install -qqy --no-install-recommends \
    wget xz-utils unzip

# Install clang + llvm version 9.0.0
RUN mkdir /usr/local/clang+llvm
WORKDIR /usr/local/clang+llvm
# --no-check-certificate is necessary due to VPN shenanigans, should remove or fix otherwise in a better environment
RUN wget --no-verbose --show-progress --progress=bar:force:noscroll --no-check-certificate \
    https://releases.llvm.org/9.0.0/clang+llvm-9.0.0-x86_64-pc-linux-gnu.tar.xz 
RUN tar -xvf clang+llvm-9.0.0-x86_64-pc-linux-gnu.tar.xz; \
    rm clang+llvm-9.0.0-x86_64-pc-linux-gnu.tar.xz; \
    ln -s /usr/local/clang+llvm/clang+llvm-9.0.0-x86_64-pc-linux-gnu/bin/opt /usr/local/clang+llvm/clang+llvm-9.0.0-x86_64-pc-linux-gnu/bin/opt-9
ENV PATH="$PATH:/usr/local/clang+llvm/clang+llvm-9.0.0-x86_64-pc-linux-gnu/bin"

# Install missing z3lib packages for clang
WORKDIR /home
# --no-check-certificate is necessary due to VPN shenanigans, should remove or fix otherwise in a better environment
RUN wget --no-verbose --show-progress --progress=bar:force:noscroll --no-check-certificate \
    https://github.com/Z3Prover/z3/releases/download/z3-4.8.17/z3-4.8.17-x64-glibc-2.31.zip
RUN unzip z3-4.8.17-x64-glibc-2.31.zip; \
    cp z3-4.8.17-x64-glibc-2.31/bin/libz3.so /usr/lib/libz3.so.4.8.17; \
    ln -s /usr/lib/libz3.so.4.8.17 /usr/lib/libz3.so.4.8

# Install wllvm (Whole-program llvm)
RUN apt-get install -qqy --no-install-recommends python3 python3-pip
RUN pip install wllvm

# Install dependencies to compile the linux kernel
RUN apt-get install -qqy --no-install-recommends \
    make flex bison libelf-dev libssl-dev gcc g++ bc file cmake