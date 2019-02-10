#!/bin/bash

gcc --static -o etc/static_server server.c
touch error.log access.log
docker build -f Dockerfile -t m2 .
rm -f error.log access.log

docker run -d --cap-drop ALL --cap-add NET_BIND_SERVICE --cap-add SETGID --cap-add SETUID\
           --cap-add SYS_CHROOT --mount type=volume,src=log,dst=/var/log/ --cpu-shares=50\
		--name "milestone2" -p 80:80 m2 
