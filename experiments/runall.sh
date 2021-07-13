#! /bin/bash
REPETITIONS=50000
DATASETFOLDER="/home/datasets"
tc qdisc replace dev docker0 root netem delay "2ms"
./start.sh --name openssl-server:1.1.1i --docker --tlsattacker --port 44441 --datasetfolder $DATASETFOLDER --dockerarguments "-v cert-data:/cert/:ro,nocopy" --clientarguments "--repetitions $REPETITIONS --skip --noskip" --serverarguments "-key /cert/rsa2048key.pem -cert /cert/rsa2048cert.pem" &
./start.sh --name openssl-0_9_7a-server --docker --tlsattacker --port 44442 --datasetfolder $DATASETFOLDER --dockerarguments "-v cert-data:/cert/:ro,nocopy" --clientarguments "--repetitions $REPETITIONS --skip --noskip" --serverarguments "-key /cert/rsa2048key.pem -cert /cert/rsa2048cert.pem" &
./start.sh --name openssl-0_9_7b-server --docker --tlsattacker --port 44443 --datasetfolder $DATASETFOLDER --dockerarguments "-v cert-data:/cert/:ro,nocopy" --clientarguments "--repetitions $REPETITIONS --skip --noskip" --serverarguments "-key /cert/rsa2048key.pem -cert /cert/rsa2048cert.pem" &
./start.sh --tag damnvulnerableopensslserver-fast --name apollolv/damnvulnerableopenssl-server --docker --tlsattacker --port 44451 --datasetfolder $DATASETFOLDER --clientarguments "--repetitions $REPETITIONS --skip --noskip" &
./start.sh --tag damnvulnerableopensslserver-full --name apollolv/damnvulnerableopenssl-server --docker --tlsattacker --port 44452 --datasetfolder $DATASETFOLDER --clientarguments "--repetitions $REPETITIONS --skip --noskip --manipulations FULL" &
./start.sh --name matrixssl-server:3.7.2 --docker --tlsattacker --port 44461 --datasetfolder $DATASETFOLDER --clientarguments "--repetitions $REPETITIONS --skip --noskip" &
./start.sh --name matrixssl-server:4.3.0 --docker --tlsattacker --port 44463 --datasetfolder $DATASETFOLDER --clientarguments "--repetitions $REPETITIONS --skip --noskip" &
./start.sh --name bouncycastle-server:1.58 --docker --tlsattacker --port 44471 --datasetfolder $DATASETFOLDER --dockerarguments "-v cert-data:/cert/:ro,nocopy" --clientarguments "--repetitions $REPETITIONS --skip --noskip" --serverarguments "4433 /cert/keys.jks password rsa2048 /cert/keys.jks password ec256" &
./start.sh --name bouncycastle-server:1.64 --docker --tlsattacker --port 44472 --datasetfolder $DATASETFOLDER --dockerarguments "-v cert-data:/cert/:ro,nocopy" --clientarguments "--repetitions $REPETITIONS --skip --noskip" --serverarguments "4433 /cert/keys.jks password rsa2048 /cert/keys.jks password ec256" &
./start.sh --name bouncycastle-server:1.57 --docker --tlsattacker --port 44473 --datasetfolder $DATASETFOLDER --dockerarguments "-v cert-data:/cert/:ro,nocopy" --clientarguments "--repetitions $REPETITIONS --skip --noskip" --serverarguments "4433 /cert/keys.jks password rsa2048 /cert/keys.jks password ec256" &
./start.sh --name bouncycastle-server:1.56 --docker --tlsattacker --port 44474 --datasetfolder $DATASETFOLDER --dockerarguments "-v cert-data:/cert/:ro,nocopy" --clientarguments "--repetitions $REPETITIONS --skip --noskip" --serverarguments "4433 /cert/keys.jks password rsa2048 /cert/keys.jks password ec256" &
./start.sh --name jssetls-jre-9.0.4-12-bc-1-59-server --docker --tlsattacker --port 44481 --datasetfolder $DATASETFOLDER --dockerarguments "-v cert-data:/cert/:ro,nocopy" --clientarguments "--repetitions $REPETITIONS --skip --noskip" --serverarguments "4433 /cert/keys.jks password rsa2048" &
./start.sh --name mbedtls-2.13.0-server --docker --tlsattacker --port 44491 --datasetfolder $DATASETFOLDER --dockerarguments "-v cert-data:/cert/:ro,nocopy" --clientarguments "--repetitions $REPETITIONS --skip --noskip" --serverarguments "crt_file=/cert/rsa2048cert.pem key_file=/cert/rsa2048key.pem server_port=4433" &
./start.sh --name mbedtls-server:2.25.0 --docker --tlsattacker --port 44494 --datasetfolder $DATASETFOLDER --dockerarguments "-v cert-data:/cert/:ro,nocopy" --clientarguments "--repetitions $REPETITIONS --skip --noskip" --serverarguments "crt_file=/cert/rsa2048cert.pem key_file=/cert/rsa2048key.pem server_port=4433" &
./start.sh --name polarssl-1.0.0-server --docker --tlsattacker --port 44492 --datasetfolder $DATASETFOLDER --dockerarguments "-v cert-data:/cert/:ro,nocopy" --clientarguments "--repetitions $REPETITIONS --skip --noskip" --serverarguments "crt_file=/cert/rsa2048cert.pem key_file=/cert/rsa2048key.pem server_port=4433" &
# docker build --build-arg VERSION=12.2-stable -t wolfssl-server:3.13.0-stable -f Dockerfile-3_0-2 --target wolfssl-server .
./start.sh --name wolfssl-server:4.4.0-stable --docker --tlsattacker --port 44601 --datasetfolder $DATASETFOLDER --dockerarguments "-v cert-data:/cert/:ro,nocopy" --clientarguments "--repetitions $REPETITIONS --skip --noskip" --serverarguments "-p 4433 -c /cert/rsa2048cert.pem -k /cert/rsa2048key.pem -i -b -l TLS_RSA_WITH_3DES_EDE_CBC_SHA:TLS_RSA_WITH_AES_128_CBC_SHA:TLS_RSA_WITH_AES_128_CBC_SHA256:TLS_RSA_WITH_AES_256_CBC_SHA256" &
./start.sh --name wolfssl-server:3.13.0-stable --docker --tlsattacker --port 44602 --datasetfolder $DATASETFOLDER --dockerarguments "-v cert-data:/cert/:ro,nocopy" --clientarguments "--repetitions $REPETITIONS --skip --noskip" --serverarguments "-p 4433 -c /cert/rsa2048cert.pem -k /cert/rsa2048key.pem -i -b -l TLS_RSA_WITH_3DES_EDE_CBC_SHA:TLS_RSA_WITH_AES_128_CBC_SHA:TLS_RSA_WITH_AES_128_CBC_SHA256:TLS_RSA_WITH_AES_256_CBC_SHA256" &
./start.sh --name wolfssl-server:3.12.2-stable --docker --tlsattacker --port 44603 --datasetfolder $DATASETFOLDER --dockerarguments "-v cert-data:/cert/:ro,nocopy" --clientarguments "--repetitions $REPETITIONS --skip --noskip" --serverarguments "-p 4433 -c /cert/rsa2048cert.pem -k /cert/rsa2048key.pem -i -b" &
./start.sh --name wolfssl-server:3.15.8 --docker --tlsattacker --port 44603 --datasetfolder $DATASETFOLDER --dockerarguments "-v cert-data:/cert/:ro,nocopy" --clientarguments "--repetitions $REPETITIONS --skip --noskip" --serverarguments "-p 4433 -c /cert/rsa2048cert.pem -k /cert/rsa2048key.pem -i -b -l TLS_RSA_WITH_AES_256_CBC_SHA256" &

# Turns out cyassl is the old wolfssl
#./start.sh --name cyassl_2.9.4-server --docker --tlsattacker --port 44602 --datasetfolder $DATASETFOLDER --clientarguments "--repetitions $REPETITIONS --skip --noskip" &
# Turns out rusttls does not support RSA key exchange
#./start.sh --name rustls --docker --tlsattacker --port 44611 --datasetfolder $DATASETFOLDER --dockerarguments "-v cert-data:/cert/:ro,nocopy" --clientarguments "--repetitions $REPETITIONS --skip --noskip" --serverarguments "--key /cert/rsa2048key.pem --certs /cert/rsa2048cert.pem --port 4433 echo" &
./start.sh --name gnutls-3_7_0-server --docker --tlsattacker --port 44621 --datasetfolder $DATASETFOLDER --dockerarguments "-v cert-data:/cert/:ro,nocopy" --clientarguments "--repetitions $REPETITIONS --skip --noskip" --serverarguments "--port=4433 --x509certfile=/cert/rsa2048cert.pem --x509keyfile=/cert/rsa2048key.pem --disable-client-cert" &
./start.sh --name boringssl-server:chromium-stable --docker --tlsattacker --port 44651 --datasetfolder $DATASETFOLDER --dockerarguments "-v cert-data:/cert/:ro,nocopy" --clientarguments "--repetitions $REPETITIONS --skip --noskip" --serverarguments "-accept 4433 -cert /cert/rsa2048cert.pem -key /cert/rsa2048key.pem -loop" &
./start.sh --name libressl-server:3.2.3 --docker --tlsattacker --port 44661 --datasetfolder $DATASETFOLDER --dockerarguments "-v cert-data:/cert/:ro,nocopy" --clientarguments "--repetitions $REPETITIONS --skip --noskip"  --serverarguments "-accept 4433 -key /cert/rsa2048key.pem -cert /cert/rsa2048cert.pem" &
./start.sh --name bearssl-server:0.6 --docker --tlsattacker --port 44671 --datasetfolder $DATASETFOLDER --dockerarguments "-v cert-data:/cert/:ro,nocopy" --clientarguments "--repetitions $REPETITIONS --skip --noskip" --serverarguments "-p 4433 -cert /cert/rsa2048cert.pem -key /cert/rsa2048key.pem" &
./start.sh --name botan-server:2.17.3 --docker --tlsattacker --port 44681 --datasetfolder $DATASETFOLDER --dockerarguments "-v cert-data:/cert/:ro,nocopy" --clientarguments "--repetitions $REPETITIONS --skip --noskip --nosni" --serverarguments "/cert/rsa2048cert.pem /cert/rsa2048key.pem --port=4433 --policy=/compat.txt" &
./start.sh --name s2n-server:0.10.25 --docker --tlsattacker --port 44671 --datasetfolder $DATASETFOLDER --dockerarguments "-v cert-data:/cert/:ro,nocopy" --clientarguments "--repetitions $REPETITIONS --skip --noskip" --serverarguments "localhost 4433 --cert /cert/rsa2048certs2n.pem --key /cert/rsa2048key.pem --parallelize" &
./start.sh --name tlslite_ng-server:0.8.0-alpha40 --docker --tlsattacker --port 44681 --datasetfolder $DATASETFOLDER --dockerarguments "-v cert-data:/cert/:ro,nocopy" --clientarguments "--repetitions $REPETITIONS --skip --noskip" --serverarguments "-c /cert/rsa2048cert.pem -k /cert/rsa2048key.pem 0.0.0.0:4433" &
./start.sh --name ocamltls-server:0.12.8 --docker --tlsattacker --port 44691 --datasetfolder $DATASETFOLDER --dockerarguments "-v cert-data:/cert/:ro,nocopy" --clientarguments "--repetitions $REPETITIONS --skip --noskip" &

# docker build -t imitation-server:5.0 -f Dockerfile .
./start.sh --name imitation-server:6.0 --tag "cisco_ace" --docker --tlsattacker --port 44501 --datasetfolder $DATASETFOLDER --clientarguments "--repetitions $REPETITIONS --noskip --wait 1500" --serverarguments "--configFile=/config/base.conf --configFile=/config/cisco_ace.conf" &
./start.sh --name imitation-server:6.0 --tag "f5_v1" --docker --tlsattacker --port 44502 --datasetfolder $DATASETFOLDER --clientarguments "--repetitions $REPETITIONS --skip --wait 1500" --serverarguments "--configFile=/config/base.conf --configFile=/config/f5_v1.conf" &
./start.sh --name imitation-server:6.0 --tag "facebook_v2" --docker --tlsattacker --port 44503 --datasetfolder $DATASETFOLDER --clientarguments "--repetitions $REPETITIONS --skip --wait 1500 --timeout 200" --serverarguments "--configFile=/config/base.conf --configFile=/config/facebook_v2.conf" &
./start.sh --name imitation-server:6.0 --tag "netscaler_gcm" --docker --tlsattacker --port 44504 --datasetfolder $DATASETFOLDER --clientarguments "--repetitions $REPETITIONS --noskip --wait 1500" --serverarguments "--configFile=/config/base.conf --configFile=/config/netscaler_gcm.conf" &
./start.sh --name imitation-server:6.0 --tag "pan_os" --docker --tlsattacker --port 44505 --datasetfolder $DATASETFOLDER --clientarguments "--repetitions $REPETITIONS --noskip --wait 1500" --serverarguments "--configFile=/config/base.conf --configFile=/config/pan_os.conf" &

REPETITIONS=2000000
./start.sh --name openssl-0_9_7a-server --docker --tlsattacker --port 44601 --datasetfolder $DATASETFOLDER --dockerarguments "-v cert-data:/cert/:ro,nocopy" --clientarguments "--repetitions $REPETITIONS --noskip --twoclass" --serverarguments "-key /cert/rsa2048key.pem -cert /cert/rsa2048cert.pem" &
./start.sh --name openssl-0_9_7b-server --docker --tlsattacker --port 44602 --datasetfolder $DATASETFOLDER --dockerarguments "-v cert-data:/cert/:ro,nocopy" --clientarguments "--repetitions $REPETITIONS --noskip --twoclass" --serverarguments "-key /cert/rsa2048key.pem -cert /cert/rsa2048cert.pem" &

wait
