FROM ubuntu:latest

ARG DEBIAN_FRONTEND=noninteractive

RUN apt-get update \
    && apt-get install -y cmake build-essential ninja-build \
    clang bison flex libreadline-dev gawk \
    tcl-dev libffi-dev git graphviz xdot pkg-config \
    libboost-system-dev libboost-filesystem-dev zlib1g-dev \
    lsb-release software-properties-common gnupg wget

WORKDIR /

RUN wget https://apt.llvm.org/llvm.sh \
    && chmod +x llvm.sh \
    && ./llvm.sh 17 \
    && rm llvm.sh

RUN git clone https://github.com/YosysHQ/yosys

WORKDIR /yosys

RUN git checkout c7d7cfeaca6c79b871aca39c0878cf189390c497 \
    && make config-clang \
    && printf "ENABLE_LIBYOSYS := 1\n" >> Makefile.conf \
    && make -j$(nproc) \
    && make install

# a workaround for yosys' abc path problem
RUN ln -s "/yosys/yosys-abc" /usr/lib/llvm-17/bin/yosys-abc
