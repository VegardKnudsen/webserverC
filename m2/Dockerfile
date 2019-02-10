FROM scratch

COPY etc/* /etc/
COPY www/* /var/www/

COPY error.log /var/log/webserver_error.log
COPY access.log /var/log/webserver_access.log

EXPOSE 80
CMD ["/etc/dumb-init", "/etc/static_server"]
