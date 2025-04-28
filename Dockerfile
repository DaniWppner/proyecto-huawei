FROM ubuntu:20.04

ARG LLVM_BUILD_JOBS=1

## Install build tool dependencies
RUN apt-get -qq update; \
    apt-get install -qqy --no-install-recommends \
    wget xz-utils unzip make gcc g++ libelf-dev libssl-dev cmake python3 python3-pip
    
# Install clang + llvm version 10.0.0
ARG LLVM_PATH=/usr/local/clang-monorepo
ARG LLVM_SOURCE_NAME=llvm-project-llvmorg-10.0.0
ARG LLVM_BUILD_PATH=${LLVM_PATH}/llvm-clang-build
ARG LLVM_SOURCE_URL=https://github.com/llvm/llvm-project/archive/refs/tags/llvmorg-10.0.0.tar.gz
ARG LLVM_SOURCE_ZIP_NAME=${LLVM_SOURCE_NAME}.tar.gz
## Download and unzip source files
RUN mkdir ${LLVM_PATH}
RUN apt-get install -qqy ca-certificates
RUN wget --no-verbose --show-progress --progress=bar:force:noscroll -O ${LLVM_PATH}/${LLVM_SOURCE_ZIP_NAME} ${LLVM_SOURCE_URL}
RUN tar -xvf ${LLVM_PATH}/${LLVM_SOURCE_ZIP_NAME} -C ${LLVM_PATH}
## Compile and make available
RUN cmake -S ${LLVM_PATH}/${LLVM_SOURCE_NAME}/llvm \
          -B ${LLVM_BUILD_PATH} \
          -DCMAKE_BUILD_TYPE=Release \
          -DLLVM_ENABLE_PROJECTS=clang
          # building all llvm projects is probably overkill
WORKDIR ${LLVM_BUILD_PATH}
RUN make -j${LLVM_BUILD_JOBS}
ENV PATH="$PATH:${LLVM_BUILD_PATH}/bin"

WORKDIR /

# Install missing zlib packages for svf-tools (included in achyb/static/libsvf)
RUN apt-get install -qqy zlib1g-dev

# Install wllvm (Whole-program llvm)
RUN pip install wllvm

# Install dependencies to compile the linux kernel
RUN apt-get install -qqy --no-install-recommends flex bison bc file 