#! /bin/bash
TOOL_FOLDER="$(pwd)"
SUT_HOST="localhost"
CAPTURE_HOST="localhost"
SUT_PORT=443
SUT_NAME="apollolv/damnvulnerableopenssl-server"
TAG=""
USE_TLS_ATTACKER=1
PARALLEL_THREADS=$(grep -c ^processor /proc/cpuinfo)
CROSSVALIDATION_TECHNIQUE="kccv"
CROSSVALIDATION_ITERATIONS=30
HYPERPARAMETER_ITERATIONS=10
CLIENT_ARGUMENTS=""
SERVER_ARGUMENTS=""
DOCKER_ARGUMENTS=""
DATASET_FOLDER=""

set -e

usage()
{
    echo 'usage example: ./start.sh --tlsattacker --tag googletest --host www.google.com --interface enp3s0 --clientarguments "--repetitions 20 --noskip"'
}

ALL_PARAMETERS=$*

while [ "$1" != "" ]; do
    case $1 in
        --name )                shift
                                SUT_NAME="$1"
                                ;;
        --tlsattacker )         USE_TLS_ATTACKER=1
                                ;;
        --port )                shift
                                SUT_PORT=$1
                                ;;
        --tag )                 shift
                                TAG=$1
                                ;;
        --datasetfolder )       shift
                                DATASET_FOLDER=$1
                                ;;
        --clientarguments )     shift
                                CLIENT_ARGUMENTS=$1
                                ;;
        --serverarguments )     shift
                                SERVER_ARGUMENTS=$1
                                ;;
        --dockerarguments )     shift
                                DOCKER_ARGUMENTS=$1
                                ;;
        --threads )             shift
                                PARALLEL_THREADS=$1
                                ;;
        -h | --help )           usage
                                exit
                                ;;
        * )                     echo "Unknown argument $1"
                                usage
                                exit 1
    esac
    shift
done

# shellcheck disable=SC2086
if [ "$TAG" ]; then
    SANITIZED_SUT_NAME=$(echo $TAG | tr -dc '[:alnum:]._-')
else
    SANITIZED_SUT_NAME=$(echo $SUT_NAME | tr -dc '[:alnum:]._-')
fi
if [ "$DATASET_FOLDER" ]; then
    FOLDER="$DATASET_FOLDER/$(date --iso-8601)-$SANITIZED_SUT_NAME"
else
    FOLDER="$TOOL_FOLDER/datasets/$(date --iso-8601)-$SANITIZED_SUT_NAME"
fi

echo "Creating dataset folder $FOLDER"
mkdir -p "$FOLDER"
cd "$FOLDER" || exit

echo "Creating setup file"
CONFIG="$FOLDER/config.md"
echo "# Experiment $SANITIZED_SUT_NAME" > "$CONFIG"
echo "## Date" >> "$CONFIG"
date >> "$CONFIG"
echo "## Host" >> "$CONFIG"
echo "$HOSTNAME" >> "$CONFIG"
echo "## Command Line Parameters" >> "$CONFIG"
echo "$ALL_PARAMETERS" >> "$CONFIG"
echo "## Output Folder" >> "$CONFIG"
echo "$FOLDER" >> "$CONFIG"
cd "$TOOL_FOLDER"
echo "## Prototype Version" >> "$CONFIG"
echo "git commit $(git rev-parse --short HEAD)" >> "$CONFIG"
echo "on branch $(git rev-parse --abbrev-ref HEAD)" >> "$CONFIG"
echo "" >> "$CONFIG"
echo "# Dataset generation" >> "$CONFIG"

echo "Starting system under test (SUT) docker image $SUT_NAME"
if [ "$(docker ps -aq -f name=$SANITIZED_SUT_NAME)" ]; then
    echo "Cleaning up a container from a previous run"
    docker stop "$SANITIZED_SUT_NAME"
fi
# shellcheck disable=SC2086
DOCKER_COMMAND="docker run -it --rm -d --name=$SANITIZED_SUT_NAME -p $SUT_PORT:4433 $DOCKER_ARGUMENTS $SUT_NAME $SERVER_ARGUMENTS"
echo "## Docker Command" >> "$CONFIG"
echo "$DOCKER_COMMAND" >> "$CONFIG"
$DOCKER_COMMAND
docker logs -f $SANITIZED_SUT_NAME 2>&1 | tee "$FOLDER/Docker Server.log" &
DOCKERLOG_PID=$!

CAPTURE_HOST=$(docker inspect -f '{{ .NetworkSettings.IPAddress }}' "$SANITIZED_SUT_NAME")
SUT_HOST="localhost"
if [ "$CAPTURE_HOST" = "" ]; then
    echo "Docker container crashed, ABORTING"
    exit 1
fi
echo "SUT started on $SUT_HOST:$SUT_PORT"

echo "## Server Hostname/IP and Port" >> "$CONFIG"
echo "$SUT_HOST:$SUT_PORT" >> "$CONFIG"

if [ "$USE_TLS_ATTACKER" = "0" ]; then
    echo "port=$SUT_PORT" > "$TOOL_FOLDER/tls_test_tool_client/config/$SANITIZED_SUT_NAME.conf"
    echo "host=$SUT_HOST" >> "$TOOL_FOLDER/tls_test_tool_client/config/$SANITIZED_SUT_NAME.conf"
fi

SUT_INTERFACE="docker0"
echo "Starting packet capture on the default docker interface $SUT_INTERFACE"

echo "Special privileges are needed to capture network traffic and add delays to interfaces"
echo "You may need to use the noroot.sh script to allow non-root users to do that"
echo "## Capturing network traffic" >> "$CONFIG"
echo "From and to $CAPTURE_HOST" >> "$CONFIG"
echo "On interface $SUT_INTERFACE" >> "$CONFIG"
tcpdump host "$CAPTURE_HOST" -U -w "$FOLDER/Packets.pcap" -i $SUT_INTERFACE &
TCPDUMP_PID=$!

if docker ps -q | grep -q "$SANITIZED_SUT_NAME"; then
    echo "Docker container crashed, ABORTING"
    exit 1
fi

if [ "$USE_TLS_ATTACKER" = "0" ]; then
    echo "Starting TLS client using the achelos TLS Test Tool"
    # shellcheck disable=SC2086
    CLIENT_COMMAND="pipenv run python3 tls_test_tool_client/client.py --folder=\"$FOLDER\" --name=$SANITIZED_SUT_NAME $CLIENT_ARGUMENTS 2>&1 | tee \"$FOLDER/TLS Test Tool Client.log\""
else
    echo "Starting TLS client using the RUB TLS Attacker"
    # shellcheck disable=SC2086
    CLIENT_COMMAND="java -jar tls_attacker_client/apps/ML-BleichenbacherGenerator.jar -connect $SUT_HOST:$SUT_PORT --folder \"$FOLDER\" $CLIENT_ARGUMENTS 2>&1 | tee \"$FOLDER/TLS Attacker Client.log\""
fi
echo "## Client Command" >> "$CONFIG"
echo "$CLIENT_COMMAND" >> "$CONFIG"
START_TIME=$(date +%s)

eval $CLIENT_COMMAND

END_TIME=$(date +%s)
DURATION="$(($END_TIME-$START_TIME))"
echo "Client finished, execution took $DURATION seconds"
echo "## Execution Time" >> "$CONFIG"
echo "$DURATION seconds" >> "$CONFIG"

echo "Stopping network capture"
sleep 1s
# shellcheck disable=SC2086
kill -2 $TCPDUMP_PID

echo "Stopping system under test docker container"
docker stop "$SANITIZED_SUT_NAME"
# kill -2 $DOCKERLOG_PID

echo "Starting feature extraction"
echo " " >> "$CONFIG"
echo "# Feature Extraction" >> "$CONFIG"
START_TIME=$(date +%s)
pipenv run python3 feature_extraction/extract.py --folder="$FOLDER" 2>&1 | tee "$FOLDER/Feature Extraction.log"
END_TIME=$(date +%s)
DURATION="$(($END_TIME-$START_TIME))"
echo "Finished feature extraction, execution took $DURATION seconds"
echo "## Execution Time" >> "$CONFIG"
echo "$DURATION seconds" >> "$CONFIG"

echo "Starting $CROSSVALIDATION_TECHNIQUE classification model training"
echo " " >> "$CONFIG"
echo "# Machine Learning" >> "$CONFIG"
echo "## Parameters" >> "$CONFIG"
echo "Using $CROSSVALIDATION_TECHNIQUE crossvalidation technique" >> "$CONFIG"
echo "Doing $CROSSVALIDATION_ITERATIONS crossvalidation iterations" >> "$CONFIG"
echo "Doing $HYPERPARAMETER_ITERATIONS hyperparameter optimization iterations" >> "$CONFIG"

START_TIME=$(date +%s)
pipenv run python3 classification_model/train_models.py --folder="$FOLDER" --cv_technique=$CROSSVALIDATION_TECHNIQUE --cv_iterations=$CROSSVALIDATION_ITERATIONS --iterations=$HYPERPARAMETER_ITERATIONS --n_jobs=$PARALLEL_THREADS 2>&1 | tee "$FOLDER/Classification Model Training $1.log"
END_TIME=$(date +%s)
DURATION="$(($END_TIME-$START_TIME))"
echo "Finished $CROSSVALIDATION_TECHNIQUE classification model training, execution took $DURATION seconds"
echo "## Execution Time" >> "$CONFIG"
echo "$DURATION seconds" >> "$CONFIG"

echo "Generating report"
pipenv run python3 classification_model/pvalues_calculation.py --folder="$FOLDER" --cv_technique=$CROSSVALIDATION_TECHNIQUE 2>&1 | tee "$FOLDER/Report Generation $1.log"

echo "Plotting the machine learning results"
pipenv run python3 classification_model/plot_results.py --folder="$FOLDER" --cv_technique=$CROSSVALIDATION_TECHNIQUE --cv_iterations=$CROSSVALIDATION_ITERATIONS 2>&1 | tee "$FOLDER/Classification Model Plotting $1.log"
echo "Finished plotting"
echo " " >> "$CONFIG"

echo "Experiment run finished" >> "$CONFIG"
echo "Experiment run finished"