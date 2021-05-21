#! /bin/bash
TOOL_FOLDER="$(pwd)"
SUT_HOST="localhost"
CAPTURE_HOST="localhost"
SUT_PORT=443
SUT_NAME="openssl-1_1_1g-server"
SUT_INTERFACE="docker0"
LATENCY=""
TAG=""
USE_TLS_ATTACKER=0
START_DOCKER=0
SKIP_LEARNING=0
ALL_PARAMETERS=""
CROSSVALIDATION_ITERATIONS=30
HYPERPARAMETER_ITERATIONS=10
CLIENT_ARGUMENTS=""
SERVER_ARGUMENTS=""
DOCKER_ARGUMENTS=""
DATASET_FOLDER=""

set -e

usage()
{
    echo 'usage example: ./start.sh --tag googletest --host www.google.com --interface enp0s3 --clientarguments "--repetitions 20 --noskip"'
}

ALL_PARAMETERS=$*

while [ "$1" != "" ]; do
    case $1 in
        --name )                shift
                                SUT_NAME="$1"
                                ;;
        --docker )              START_DOCKER=1
                                ;;
        --tlsattacker )         USE_TLS_ATTACKER=1
                                ;;
        --host )                shift
                                SUT_HOST=$1
                                ;;
        --port )                shift
                                SUT_PORT=$1
                                ;;
        --interface )           shift
                                SUT_INTERFACE=$1
                                ;;
        --latency )             shift
                                LATENCY=$1
                                ;;
        --tag )                 shift
                                TAG=$1
                                ;;
        --datasetfolder )       shift
                                DATASET_FOLDER=$1
                                ;;
        --skiplearning )        SKIP_LEARNING=1
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
    SANITIZED_SUT_NAME=$(echo $TAG | tr -dc '[:alnum:]')
else
    SANITIZED_SUT_NAME=$(echo $SUT_NAME | tr -dc '[:alnum:]')
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

if [ "$START_DOCKER" = "1" ]; then
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
else
    echo "Assuming the system under test (SUT) is already running at $SUT_HOST:$SUT_PORT on interface $SUT_INTERFACE"
fi

echo "## Server Hostname/IP and Port" >> "$CONFIG"
echo "$SUT_HOST:$SUT_PORT" >> "$CONFIG"

if [ "$USE_TLS_ATTACKER" = "0" ]; then
    echo "port=$SUT_PORT" > "$TOOL_FOLDER/scriptable_client/config/$SANITIZED_SUT_NAME.conf"
    echo "host=$SUT_HOST" >> "$TOOL_FOLDER/scriptable_client/config/$SANITIZED_SUT_NAME.conf"
fi

echo "Special privileges are needed to capture network traffic and add delays to interfaces"
echo "You may need to use the noroot.sh script to allow non-root users to do that"
echo "Capturing on interface $SUT_INTERFACE"
echo "## Capturing network traffic" >> "$CONFIG"
echo "From and to $CAPTURE_HOST" >> "$CONFIG"
echo "On interface $SUT_INTERFACE" >> "$CONFIG"
tcpdump host "$CAPTURE_HOST" -U -w "$FOLDER/Packets.pcap" -i $SUT_INTERFACE &
TCPDUMP_PID=$!

if [ "$LATENCY" ]; then
    echo "Adding an artificial latency of $LATENCY to the interface $SUT_INTERFACE"
    tc qdisc replace dev $SUT_INTERFACE root netem delay "$LATENCY"
fi

if [ "$START_DOCKER" = "1" ]; then
    if docker ps -q | grep -q "$SANITIZED_SUT_NAME"; then
        echo "Docker container crashed, ABORTING"
        exit 1
    fi
fi

if [ "$USE_TLS_ATTACKER" = "0" ]; then
    echo "Starting TLS client using the Achelos TLS Test Tool"
    cd "$TOOL_FOLDER/scriptable_client" || exit
    # shellcheck disable=SC2086
    CLIENT_COMMAND="pipenv run python3 client.py --folder=\"$FOLDER\" --name=$SANITIZED_SUT_NAME $CLIENT_ARGUMENTS 2>&1 | tee \"$FOLDER/Scriptable Client.log\""
else
    echo "Starting TLS client using the RUB TLS Attacker"
    cd "$TOOL_FOLDER/tls_attacker_client" || exit
    # shellcheck disable=SC2086
    CLIENT_COMMAND="java -jar apps/ML-BleichenbacherGenerator.jar -connect $SUT_HOST:$SUT_PORT --folder \"$FOLDER\" $CLIENT_ARGUMENTS 2>&1 | tee \"$FOLDER/TLS Attacker.log\""
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

if [ "$LATENCY" ]; then
    echo "Removing network delay"
    tc qdisc delete dev $SUT_INTERFACE root
fi

if [ "$START_DOCKER" = "1" ]; then
    echo "Stopping system under test docker container"
    docker stop "$SANITIZED_SUT_NAME"
    # kill -2 $DOCKERLOG_PID
fi

echo "Starting feature extraction"
echo "# Feature Extraction" >> "$CONFIG"
START_TIME=$(date +%s)
cd "$TOOL_FOLDER/feature_extraction" || exit
pipenv run python3 extract.py --folder="$FOLDER" 2>&1 | tee "$FOLDER/Feature Extraction.log"
END_TIME=$(date +%s)
DURATION="$(($END_TIME-$START_TIME))"
echo "Finished feature extraction, execution took $DURATION seconds"
echo "## Execution Time" >> "$CONFIG"
echo "$DURATION seconds" >> "$CONFIG"

if [ "$SKIP_LEARNING" = "0" ]; then
    echo "Starting classification model training"
    cd "$TOOL_FOLDER/classification_model" || exit
    echo "# Machine Learning" >> "$CONFIG"
    echo "## Parameters" >> "$CONFIG"
    echo "Doing $CROSSVALIDATION_ITERATIONS crossvalidation iterations" >> "$CONFIG"
    echo "Doing $HYPERPARAMETER_ITERATIONS hyperparameter optimization iterations" >> "$CONFIG"

    START_TIME=$(date +%s)
    pipenv run python3 train_models.py --folder="$FOLDER" --cv_iterations=$CROSSVALIDATION_ITERATIONS --iterations=$HYPERPARAMETER_ITERATIONS 2>&1 | tee "$FOLDER/Classification Model Training.log"
    END_TIME=$(date +%s)
    DURATION="$(($END_TIME-$START_TIME))"
    echo "Finished classification model training, execution took $DURATION seconds"
    echo "## Execution Time" >> "$CONFIG"
    echo "$DURATION seconds" >> "$CONFIG"

    echo "Plotting the machine learning results"
    pipenv run python3 plot_results.py --folder="$FOLDER" --cv_iterations=$CROSSVALIDATION_ITERATIONS 2>&1 | tee "$FOLDER/Classification Model Plotting.log"
    echo "Finished plotting"
fi