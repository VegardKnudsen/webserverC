FROM scratch

COPY static_server /static_server
COPY dumb-init /dumb-init

COPY index.asis /var/www/index.asis
COPY error.log /var/log/webserver_error.log

EXPOSE 8080

CMD ["/dumb-init", "/static_server"]
