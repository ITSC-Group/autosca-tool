#! /bin/bash
TOOL_FOLDER="$(pwd)"
SUT_HOST="www.google.com"
CAPTURE_HOST=""
SUT_PORT=443
SUT_INTERFACE=$(ip route | grep '^default' | cut -d' ' -f5)
USE_TLS_ATTACKER=1
CLIENT_ARGUMENTS=""
FOLDER="/home/datasets/$(date --iso-8601)-unnamed"

set -e

usage()
{
    echo 'usage example: ./generate_remote_server_dataset.sh --host www.google.com --port 443 --folder $HOME/experiment1 --clientarguments "--repetitions 2000 --noskip"'
}

ALL_PARAMETERS=$*

while [ "$1" != "" ]; do
    case $1 in
        --host )                shift
                                SUT_HOST=$1
                                ;;
        --port )                shift
                                SUT_PORT=$1
                                ;;
        --folder )              shift
                                FOLDER=$1
                                ;;
        --interface )           shift
                                SUT_INTERFACE=$1
                                ;;
        --tlsattacker )         USE_TLS_ATTACKER=1
                                ;;
        --tlstesttool )         USE_TLS_ATTACKER=0
                                ;;
        --clientarguments )     shift
                                CLIENT_ARGUMENTS=$1
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
echo "# Experiment $SANITIZED_SUT_NAME" > "$CONFIG"
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

SANITIZED_SUT_NAME=$(echo "$SUT_HOST"| tr -dc '[:alnum:]._-')

echo "## Server Hostname/IP and Port" >> "$CONFIG"
echo "$SUT_HOST:$SUT_PORT" >> "$CONFIG"

if [ "$USE_TLS_ATTACKER" = "0" ]; then
    echo "port=$SUT_PORT" > "$TOOL_FOLDER/tls_test_tool_client/config/$SANITIZED_SUT_NAME.conf"
    echo "host=$SUT_HOST" >> "$TOOL_FOLDER/tls_test_tool_client/config/$SANITIZED_SUT_NAME.conf"
fi

CAPTURE_HOST=$SUT_HOST
echo "Starting packet capture on $SUT_INTERFACE"

echo "Special privileges are needed to capture network traffic and add delays to interfaces"
echo "You may need to use the noroot.sh script to allow non-root users to do that"
echo "## Capturing network traffic" >> "$CONFIG"
echo "From and to $CAPTURE_HOST" >> "$CONFIG"
echo "On interface $SUT_INTERFACE" >> "$CONFIG"
tcpdump host "$CAPTURE_HOST" -U -w "$FOLDER/Packets.pcap" -i $SUT_INTERFACE &
TCPDUMP_PID=$!

echo "Connecting to TLS server running on $SUT_HOST:$SUT_PORT"
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

echo "Dataset generation finished" >> "$CONFIG"
echo "Dataset generation finished"