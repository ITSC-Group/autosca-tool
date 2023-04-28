REPETITIONS=2200000
FOLDER=/home/datasets/$(date --iso-8601)-journal/$(date --iso-8601)-OpenSSL097a
./generate_docker_dataset.sh --image openssl-0_9_7a-server --port 44610 --folder "$FOLDER" --tlsattacker --dockerarguments "-v cert-data:/cert/:ro,nocopy" --clientarguments "--repetitions $REPETITIONS" --serverarguments "-key /cert/rsa2048key.pem -cert /cert/rsa2048cert.pem" && pipenv run python3 feature_extraction/extract.py --folder="$FOLDER" 2>&1 | tee "$FOLDER/Feature Extraction.log" &
FOLDER=/home/datasets/$(date --iso-8601)-journal/$(date --iso-8601)-OpenSSL097b
./generate_docker_dataset.sh --image openssl-0_9_7b-server --port 44611 --folder "$FOLDER" --tlsattacker --dockerarguments "-v cert-data:/cert/:ro,nocopy" --clientarguments "--repetitions $REPETITIONS" --serverarguments "-key /cert/rsa2048key.pem -cert /cert/rsa2048cert.pem" && pipenv run python3 feature_extraction/extract.py --folder="$FOLDER" 2>&1 | tee "$FOLDER/Feature Extraction.log" &
FOLDER=/home/datasets/$(date --iso-8601)-journal/$(date --iso-8601)-OpenSSL111t
./generate_docker_dataset.sh --image openssl-server:1.1.1t --port 44612 --folder "$FOLDER" --tlsattacker --dockerarguments "-v cert-data:/cert/:ro,nocopy" --clientarguments "--repetitions $REPETITIONS" --serverarguments "-key /cert/rsa2048key.pem -cert /cert/rsa2048cert.pem" && pipenv run python3 feature_extraction/extract.py --folder="$FOLDER" 2>&1 | tee "$FOLDER/Feature Extraction.log" &
FOLDER=/home/datasets/$(date --iso-8601)-journal/$(date --iso-8601)-DamnVulnerableOpenSSL
./generate_docker_dataset.sh --image damnvulnerableopenssl-server --port 44613 --folder "$FOLDER" --tlsattacker --dockerarguments "-v cert-data:/cert/:ro,nocopy" --clientarguments "--repetitions $REPETITIONS" --serverarguments "-key /cert/rsa2048key.pem -cert /cert/rsa2048cert.pem" && pipenv run python3 feature_extraction/extract.py --folder="$FOLDER" 2>&1 | tee "$FOLDER/Feature Extraction.log" &
FOLDER=/home/datasets/$(date --iso-8601)-journal/$(date --iso-8601)-cisco_ace
./generate_docker_dataset.sh --image imitation-server:8.0 --port 44621 --folder "$FOLDER" --dockercontainername "cisco_ace" --tlsattacker  --clientarguments "--repetitions $REPETITIONS --wait 1500 --timeout 200" --serverarguments "--configFile=/config/base.conf --configFile=/config/cisco_ace.conf" && pipenv run python3 feature_extraction/extract.py --folder="$FOLDER" 2>&1 | tee "$FOLDER/Feature Extraction.log" &
FOLDER=/home/datasets/$(date --iso-8601)-journal/$(date --iso-8601)-f5_v1
./generate_docker_dataset.sh --image imitation-server:8.0 --port 44622 --folder "$FOLDER" --dockercontainername "f5_v1" --tlsattacker  --clientarguments "--repetitions $REPETITIONS --wait 1500 --timeout 200" --serverarguments "--configFile=/config/base.conf --configFile=/config/f5_v1.conf" && pipenv run python3 feature_extraction/extract.py --folder="$FOLDER" 2>&1 | tee "$FOLDER/Feature Extraction.log" &
FOLDER=/home/datasets/$(date --iso-8601)-journal/$(date --iso-8601)-facebook_v2
./generate_docker_dataset.sh --image imitation-server:8.0 --port 44623 --folder "$FOLDER" --dockercontainername "facebook_v2" --tlsattacker  --clientarguments "--repetitions $REPETITIONS --wait 1500 --timeout 200" --serverarguments "--configFile=/config/base.conf --configFile=/config/facebook_v2.conf" && pipenv run python3 feature_extraction/extract.py --folder="$FOLDER" 2>&1 | tee "$FOLDER/Feature Extraction.log" &
FOLDER=/home/datasets/$(date --iso-8601)-journal/$(date --iso-8601)-netscaler_gcm
./generate_docker_dataset.sh --image imitation-server:8.0 --port 44624 --folder "$FOLDER" --dockercontainername "netscaler_gcm" --tlsattacker  --clientarguments "--repetitions $REPETITIONS --wait 1500 --timeout 200" --serverarguments "--configFile=/config/base.conf --configFile=/config/netscaler_gcm.conf" && pipenv run python3 feature_extraction/extract.py --folder="$FOLDER" 2>&1 | tee "$FOLDER/Feature Extraction.log" &
FOLDER=/home/datasets/$(date --iso-8601)-journal/$(date --iso-8601)-pan_os
./generate_docker_dataset.sh --image imitation-server:8.0 --port 44625 --folder "$FOLDER" --dockercontainername "pan_os" --tlsattacker  --clientarguments "--repetitions $REPETITIONS --wait 1500 --timeout 200" --serverarguments "--configFile=/config/base.conf --configFile=/config/pan_os.conf" && pipenv run python3 feature_extraction/extract.py --folder="$FOLDER" 2>&1 | tee "$FOLDER/Feature Extraction.log" &