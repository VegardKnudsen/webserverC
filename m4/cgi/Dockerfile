FROM httpd:alpine

COPY addbook.cgi /usr/local/apache2/cgi/
COPY httpd.conf /usr/local/apache2/conf/httpd.conf

RUN apk update
RUN apk add curl

EXPOSE 80