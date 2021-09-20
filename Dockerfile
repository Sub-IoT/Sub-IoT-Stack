FROM ubuntu:20.04

RUN apt-get update
RUN apt-get upgrade -y
RUN apt-get install -y python3-pip libtool srecord wget git
COPY . /opt/sub-iot-stack
RUN tar xf /opt/sub-iot-stack/gcc-arm-none-eabi-8-2018-q4-major-linux.tar.bz2 -C /opt
RUN rm /opt/sub-iot-stack/gcc-arm-none-eabi-8-2018-q4-major-linux.tar.bz2
RUN pip3 install cmake
RUN apt-get autoclean -y
RUN apt-get autoremove -y
RUN apt-get clean

ENV PATH=/opt/gcc-arm-none-eabi-8-2018-q4-major/bin:$PATH

