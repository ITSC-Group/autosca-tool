#! /bin/bash
# Non-parallelized skip experiment
#./start.sh --docker --name openssl-1_1_1g-server --tag nonparallelskip --port 4433 --latency 10ms --clientarguments --repetitions 10 --skip --processes 1
# Parallelized skip experiment
#./start.sh --docker --name openssl-1_1_1g-server --tag parallelskip --port 4433 --latency 10ms --clientarguments --repetitions 10 --skip --processes 4
# 3000er dataset non-vulnerable
#./start.sh --docker --name openssl-1_1_1g-server --port 4433 --latency 10ms --clientarguments --repetitions 3000 --noskip
# 3000er dataset vulnerable
#./start.sh --docker --name apollolv/damnvulnerableopenssl-server --port 4433 --latency 10ms --clientarguments --repetitions 3000 --noskip
# 300er dataset machine learning failure
#./start.sh --docker --name apollolv/damnvulnerableopenssl-server --tag learningfailure --port 4433 --latency 10ms --clientarguments --repetitions 300 --noskip
# quick functionality test
#./start.sh --docker --name apollolv/damnvulnerableopenssl-server --tag quicktest --port 4433 --latency 10ms --clientarguments --repetitions 20 --noskip
# external host test
#./start.sh --tag googletest --host www.google.com --interface enp0s3 --clientarguments --repetitions 20 --noskip --wait 1 --processes 1
# Skip and Noskip quicktest
#./start.sh --tag skiptest --docker --name openssl-1_1_1e-server --port 4433 --latency 10ms --clientarguments --repetitions 20 --skip --noskip --processes 1
# 6000er dataset non-vulnerable including skipping
#./start.sh --tag opensslskipping --docker --name openssl-1_1_1e-server --port 4433 --latency 10ms --clientarguments --repetitions 6000 --noskip --skip --processes 1
# 6000er dataset vulnerable including skipping
#./start.sh --tag vulnerableskipping --docker --name apollolv/damnvulnerableopenssl-server --port 4433 --latency 10ms --clientarguments --repetitions 6000 --noskip --skip --processes 1
# MatrixSSL 3.4.0 CVE 2016-6883 dataset
#./start.sh --docker --name matrixssl-3-7.2-server --port 4433 --latency 10ms --skipvolume --clientarguments --repetitions 10000 --noskip
# OpenSSL 0.9.7a Klima-Pokorny-Rosa dataset
#./start.sh --docker --name openssl-0_9_7a-server --port 4433 --latency 10ms --clientarguments --repetitions 10000 --skip --noskip --processes 1
# DamnvulnerableOpenSSL dataset
#./start.sh --docker --name apollolv/damnvulnerableopenssl-server --port 4433 --latency 10ms --clientarguments --repetitions 10000 --skip --noskip --processes 1
# OpenSSL 1.1.1g dataset
#./start.sh --docker --name openssl-1_1_1g-server --port 4433 --latency 10ms --clientarguments --repetitions 10000 --skip --noskip --processes 1
# TLS attacker test
#./start.sh --docker --tlsattacker --name openssl-1_1_1g-server --port 4433 --latency 5ms --skiplearning --clientarguments "--repetitions 300 --skip --noskip" --serverarguments "-key /cert/rsa2048key.pem -cert /cert/rsa2048cert.pem"
# Parallel docker port assignment test
tc qdisc replace dev docker0 root netem delay "5ms"
./start.sh --name openssl-1_1_1g-server --docker --tlsattacker --skiplearning --port 44444 --clientarguments "--repetitions 100 --skip --noskip" --serverarguments "-key /cert/rsa2048key.pem -cert /cert/rsa2048cert.pem" &
./start.sh --name openssl-0_9_7a-server --docker --tlsattacker --skiplearning --port 33333 --clientarguments "--repetitions 100 --skip --noskip" --serverarguments "-key /cert/rsa2048key.pem -cert /cert/rsa2048cert.pem"
tc qdisc delete dev docker0 root
