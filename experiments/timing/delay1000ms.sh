#! /bin/bash
DATASET_FOLDER="/home/datasets"
BEGIN_INDEX=1
REPETITIONS=50
SUT_IMAGE="apollolv/damnvulnerableopenssl-server:delay1000ms"
ANALYSIS_IMAGE="itscgroup/autosca-analysis:latest"

PARENT_FOLDER="$DATASET_FOLDER/$(date --iso-8601)-delay1000ms-$REPETITIONS"
echo "Creating dataset folder $PARENT_FOLDER"
mkdir -p "$PARENT_FOLDER"

scan_container(){
    INDEX=$1
    SUMMARY_FILE="$PARENT_FOLDER/Summary.md"
    CHILD_FOLDER="$PARENT_FOLDER/$INDEX"
    mkdir -p "$CHILD_FOLDER"
    echo "Executing scan number $INDEX";
    ./generate_docker_dataset.sh --image $SUT_IMAGE --port 44505 --folder $CHILD_FOLDER --clientarguments "--repetitions 2000 --noskip --timeout 1050"
    docker run --mount type=bind,source=$DATASET_FOLDER,target=$DATASET_FOLDER $ANALYSIS_IMAGE --folder $CHILD_FOLDER

    echo -e "\n\n# $INDEX $DOMAIN" >> "$SUMMARY_FILE"
    if [ -f "$CHILD_FOLDER/Report.txt" ]; then
        cat "$CHILD_FOLDER/Report.txt" >> "$SUMMARY_FILE"

    elif [ -f "$CHILD_FOLDER/TLS Attacker.log" ]; then
        echo "## TLS Attacker Log" >> "$SUMMARY_FILE"
        head -n 2 "$CHILD_FOLDER/TLS Attacker.log" >> "$SUMMARY_FILE"
        if [ -f "$CHILD_FOLDER/Classification Model Training.log" ]; then
            echo "## Classification Model Training Log" >> "$SUMMARY_FILE"
            tail -n 3 "$CHILD_FOLDER/Classification Model Training.log" >> "$SUMMARY_FILE"
        fi

    else
        echo "Experiment crashed, no output files found" >> "$SUMMARY_FILE"
    fi
}

# initialize a semaphore with a given number of tokens
open_sem(){  
    mkfifo pipe-$$
    exec 3<>pipe-$$
    rm pipe-$$
    local i=$1
    for((;i>0;i--)); do
        printf %s 000 >&3
    done
}

# run the given command asynchronously and pop/push tokens
run_with_lock(){
    local x
    # this read waits until there is something to read
    read -u 3 -n 3 x && ((0==x)) || exit $x
    (
     ( "$@"; )
    # push the return code of the command to the semaphore
    printf '%.3d' $? >&3
    )&
}

N=1 # Number of semaphores, equivalent to the number of parallel executions of the scan
open_sem $N
x=$BEGIN_INDEX
while [ $x -le $REPETITIONS ]
do
    run_with_lock scan_container "$x"
    x=$(( $x + 1 ))
done

wait