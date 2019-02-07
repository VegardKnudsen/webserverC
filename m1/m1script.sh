#!/bin/bash

gcc --static -o static_server server.c
touch error.log
docker build -f Dockerfile -t m1 .
rm -f error.log

docker run -d --cap-drop ALL --cap-add NET_BIND_SERVICE --cap-add SETGID --cap-add SETUID\
           --cap-add SYS_CHROOT --mount type=volume,src=log,dst=/var/log/ --cpu-shares=50\
		--name "milestone1" -p 8080:8080 m1

#docker run --name "milestone1" -p 80:8080 m1
