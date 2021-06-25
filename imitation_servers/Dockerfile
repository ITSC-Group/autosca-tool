# imitationtls-server:7.0
FROM debian:latest
ADD config /config
ADD data /data
ADD TlsTestTool /bin/TlsTestTool
RUN chmod +x /bin/TlsTestTool
EXPOSE 4433
ENTRYPOINT ["TlsTestTool"]