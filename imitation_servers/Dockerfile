# imitationtls-server:6.0
FROM debian:latest
ADD config /config
ADD data /data
ADD TlsTestTool /bin/TlsTestTool
EXPOSE 4433
ENTRYPOINT ["TlsTestTool"]