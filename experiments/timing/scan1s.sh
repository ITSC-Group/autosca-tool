#! /bin/bash
REPETITIONS=2
DATASET_FOLDER="/home/datasets"
BEGIN_INDEX=1
INDEX_AMOUNT=50
DOMAIN="imitation-server:7.2"

FOLDER="$DATASET_FOLDER/$(date --iso-8601)-1sdelay-$INDEX_AMOUNT"
echo "Creating dataset folder $FOLDER"
mkdir -p "$FOLDER"

scan_domain(){
        INDEX=$1
        SUMMARY="$FOLDER/Summary.md"
        echo "Executing scan number $INDEX";
        ./start.sh --name $DOMAIN --tag $INDEX --docker --tlsattacker --port 44505 --datasetfolder $FOLDER --clientarguments "--repetitions $REPETITIONS --noskip --wait 1500" --serverarguments "--configFile=/config/base.conf --configFile=/config/time_delay_1s.conf"

        echo -e "\n\n# $INDEX $DOMAIN" >> "$SUMMARY"
        if [ -f "$FOLDER/$INDEX$DOMAIN/Report.txt" ]; then
            cat "$FOLDER/$INDEX$DOMAIN/Report.txt" >> "$SUMMARY"

        elif [ -f "$FOLDER/$INDEX$DOMAIN/TLS Attacker.log" ]; then
            echo "## TLS Attacker Log" >> "$SUMMARY"
            head -n 2 "$FOLDER/$INDEX$DOMAIN/TLS Attacker.log" >> "$SUMMARY"
            if [ -f "$FOLDER/$INDEX$DOMAIN/Classification Model Training.log" ]; then
                echo "## Classification Model Training Log" >> "$SUMMARY"
                tail -n 3 "$FOLDER/$INDEX$DOMAIN/Classification Model Training.log" >> "$SUMMARY"
            fi

        else
            echo "Experiment crashed, no output files found" >> "$SUMMARY"
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

N=1
open_sem $N
x=1
while [ $x -le $INDEX_AMOUNT ]
do
    run_with_lock scan_domain "$x"
    x=$(( $x + 1 ))
done

wait