#! /bin/bash
killall TlsTestTool
killall tcpdump
# shellcheck disable=SC2046
docker kill $(docker ps -q)
tc qdisc delete dev docker0 root