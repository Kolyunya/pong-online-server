FROM debian:wheezy
MAINTAINER Nikolay Oleynikov

RUN echo "deb http://http.debian.net/debian wheezy-backports main" >> /etc/apt/sources.list
RUN apt-get update
RUN apt-get upgrade -y
RUN apt-get install -y qtbase5-dev
COPY PongOnlineServer/Build/Release/PongOnlineServer /var/pong-online-server/
COPY PongOnlineServer/PongOnlineServer.ini /var/pong-online-server/

CMD /var/pong-online-server/PongOnlineServer

