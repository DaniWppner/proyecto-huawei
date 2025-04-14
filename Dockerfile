FROM ubuntu:20.04

# Install dependencies
RUN apt-get -qq update; \
    apt-get install -qqy --no-install-recommends \
    wget xz-utils unzip

# Install clang + llvm version 9.0.0
RUN mkdir /usr/local/clang+llvm
WORKDIR /usr/local/clang+llvm
RUN wget --no-verbose --show-progress --progress=bar:force:noscroll \
    https://releases.llvm.org/9.0.0/clang+llvm-9.0.0-x86_64-pc-linux-gnu.tar.xz 
RUN tar -xvf clang+llvm-9.0.0-x86_64-pc-linux-gnu.tar.xz; \
    rm clang+llvm-9.0.0-x86_64-pc-linux-gnu.tar.xz; \
    ln -s /usr/local/clang+llvm/clang+llvm-9.0.0-x86_64-pc-linux-gnu/bin/opt /usr/local/clang+llvm/clang+llvm-9.0.0-x86_64-pc-linux-gnu/bin/opt-9
ENV PATH="$PATH:/usr/local/clang+llvm/clang+llvm-9.0.0-x86_64-pc-linux-gnu/bin"
WORKDIR /home

# Install missing zlib packages for svf-tools (included in achyb/static/libsvf)
RUN apt-get install -qqy zlib1g-dev

# Install build tool dependencies
RUN apt-get install -qqy make gcc g++ libelf-dev libssl-dev cmake

# Install wllvm (Whole-program llvm)
RUN apt-get install -qqy --no-install-recommends python3 python3-pip
RUN pip install wllvm

# Install dependencies to compile the linux kernel
RUN apt-get install -qqy --no-install-recommends \
    flex bison bc file 