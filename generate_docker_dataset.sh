#! /bin/bash
TOOL_FOLDER="$(pwd)"
SUT_HOST="localhost"
CAPTURE_HOST="localhost"
SUT_PORT=443
SUT_NAME="apollolv/damnvulnerableopenssl-server"
SUT_INTERFACE="docker0"
USE_TLS_ATTACKER=1
CLIENT_ARGUMENTS=""
SERVER_ARGUMENTS=""
DOCKER_ARGUMENTS=""
DOCKER_CONTAINER_NAME=""
FOLDER="/home/datasets/$(date --iso-8601)-unnamed"

set -e

usage()
{
    echo 'usage example: ./generate_docker_dataset.sh --image apollolv/damnvulnerableopenssl-server --port 44701 --folder $HOME/experiment1 --clientarguments "--repetitions 2000 --noskip"'
}

ALL_PARAMETERS=$*

while [ "$1" != "" ]; do
    case $1 in
        --image )               shift
                                SUT_NAME="$1"
                                ;;
        --tlsattacker )         USE_TLS_ATTACKER=1
                                ;;
        --tlstesttool )         USE_TLS_ATTACKER=0
                                ;;
        --port )                shift
                                SUT_PORT=$1
                                ;;
        --folder )              shift
                                FOLDER=$1
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
        --dockercontainername ) shift
                                DOCKER_CONTAINER_NAME=$1
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

echo "Creating dataset folder $FOLDER"
mkdir -p "$FOLDER"
cd "$FOLDER" || exit

echo "Creating setup file"
CONFIG="$FOLDER/config.md"
echo "# Experiment $TEST_TOOL_CONFIG_FILE_NAME" > "$CONFIG"
echo "## Date" >> "$CONFIG"
date >> "$CONFIG"
echo "## Host" >> "$CONFIG"
echo "$HOSTNAME" >> "$CONFIG"
echo "## User" >> "$CONFIG"
echo "$USER" >> "$CONFIG"
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

if [ -z "${DOCKER_CONTAINER_NAME}" ]; then
    DOCKER_CONTAINER_NAME=$(echo "$SUT_NAME" | tr -dc '[:alnum:]._-')
fi
echo "Starting system under test (SUT) docker image $SUT_NAME"
if [ "$(docker ps -aq -f name=$DOCKER_CONTAINER_NAME)" ]; then
    echo "Cleaning up a container from a previous run"
    docker stop "$DOCKER_CONTAINER_NAME"
fi
# shellcheck disable=SC2086
DOCKER_COMMAND="docker run -it --rm -d --name=$DOCKER_CONTAINER_NAME -p $SUT_PORT:4433 $DOCKER_ARGUMENTS $SUT_NAME $SERVER_ARGUMENTS"
echo "## Docker Command" >> "$CONFIG"
echo "$DOCKER_COMMAND" >> "$CONFIG"
$DOCKER_COMMAND
DOCKERLOG_PID=$!

CAPTURE_HOST=$(docker inspect -f '{{ .NetworkSettings.IPAddress }}' "$DOCKER_CONTAINER_NAME")
SUT_HOST="localhost"
if [ "$CAPTURE_HOST" = "" ]; then
    echo "Docker container crashed, ABORTING"
    exit 1
fi
echo "SUT started on $SUT_HOST:$SUT_PORT"

echo "## Server Hostname/IP and Port" >> "$CONFIG"
echo "$SUT_HOST:$SUT_PORT" >> "$CONFIG"

if [ "$USE_TLS_ATTACKER" = "0" ]; then
    TEST_TOOL_CONFIG_FILE_NAME=$(echo "$SUT_NAME" | tr -dc '[:alnum:]._-')
    echo "port=$SUT_PORT" > "$TOOL_FOLDER/tls_test_tool_client/config/$TEST_TOOL_CONFIG_FILE_NAME.conf"
    echo "host=$SUT_HOST" >> "$TOOL_FOLDER/tls_test_tool_client/config/$TEST_TOOL_CONFIG_FILE_NAME.conf"
fi

echo "Starting packet capture on the default docker interface $SUT_INTERFACE"

echo "Special privileges are needed to capture network traffic and add delays to interfaces"
echo "You may need to use the noroot.sh script to allow non-root users to do that"
echo "## Capturing network traffic" >> "$CONFIG"
echo "From and to $CAPTURE_HOST" >> "$CONFIG"
echo "On interface $SUT_INTERFACE" >> "$CONFIG"
tcpdump host "$CAPTURE_HOST" -U -w "$FOLDER/Packets.pcap" -i $SUT_INTERFACE &
TCPDUMP_PID=$!

if docker ps -q | grep -q "$DOCKER_CONTAINER_NAME"; then
    echo "Docker container crashed, ABORTING"
    exit 1
fi

if [ "$USE_TLS_ATTACKER" = "0" ]; then
    echo "Starting TLS client using the achelos TLS Test Tool"
    # shellcheck disable=SC2086
    CLIENT_COMMAND="pipenv run python3 tls_test_tool_client/client.py --folder=\"$FOLDER\" --name=$TEST_TOOL_CONFIG_FILE_NAME $CLIENT_ARGUMENTS 2>&1 | tee \"$FOLDER/TLS Test Tool Client.log\""
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
docker stop "$DOCKER_CONTAINER_NAME"
# kill -2 $DOCKERLOG_PID

echo "Dataset generation finished" >> "$CONFIG"
echo "Dataset generation finished"
