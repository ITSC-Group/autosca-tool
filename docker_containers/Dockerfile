# autosca-tool:0.1
FROM debian:latest
ADD .. /autosca-tool
RUN apt-get update
RUN apt-get upgrade
RUN apt-get install python3 pip3 libxml2-dev libxslt-dev tcpdump tshark default-jre maven cmake
RUN bash /autosca-tool/setup.sh
ENTRYPOINT ["bash /autosca-tool/analyze_dataset.sh"]