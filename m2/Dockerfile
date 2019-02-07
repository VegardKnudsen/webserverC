FROM scratch

COPY etc/* /etc/
COPY www/* /var/www/

COPY error.log /var/log/webserver_error.log
COPY access.log /var/log/webserver_access.log

EXPOSE 8080
CMD ["/etc/dumb-init", "/etc/static_server"]
