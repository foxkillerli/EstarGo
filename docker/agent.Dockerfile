FROM ubuntu:18.04
RUN apt-get update
RUN apt-get install -y build-essential \
    ssh \
    gcc \
    g++ \
    gdb \
    clang \
    cmake \
    rsync \
    tar \
    python \
    libgflags-dev \
    liblog4cplus-dev
RUN apt-get install -y openssh-server
RUN mkdir /var/run/sshd
RUN sed -ri 's/^PermitRootLogin\s+.*/PermitRootLogin yes/g' /etc/ssh/sshd_config
RUN sed -ri 's/UsePAM yes/#UsePAM no/g' /etc/ssh/sshd_config

RUN apt-get install -y rsync
RUN sed -ri 's/RSYNC_ENABLE=false/RSYNC_ENABLE=true/g' /etc/default/rsync
COPY rsync.conf /etc

RUN useradd -m user && yes password | passwd user

RUN mkdir /root/sync

COPY entrypoint.sh /sbin
RUN chmod +x /sbin/entrypoint.sh
ENTRYPOINT [ "/sbin/entrypoint.sh" ]
