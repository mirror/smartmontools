FROM ubuntu:14.04.5
MAINTAINER Allan Liu <allanliu55@gmail.com>

ARG login_user=test

RUN localedef -i en_US -f UTF-8 en_US.UTF-8

RUN apt-get update && \
 apt-get install -y git && \
 apt-get install -y python && \
 apt-get install -y automake && \
 apt-get install -y build-essential

RUN adduser --disabled-password --gecos '' $login_user && \
  adduser $login_user sudo && \
  exec echo '%sudo ALL=(ALL) NOPASSWD:ALL' >> /etc/sudoers

USER $login_user

CMD ["bin/bash"]
