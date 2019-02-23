#!/bin/bash

docker container rm cgiserver

docker build --rm -t nan/mp3 .

docker run -it -p 8080:80 --name cgiserver nan/mp3