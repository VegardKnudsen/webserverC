#!/bin/bash

gcc --static -o static_server server.c
docker build -t myserver .
docker run -p 8080:80 myserver
