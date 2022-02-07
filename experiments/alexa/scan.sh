#! /bin/bash
REPETITIONS=2000
DATASET_FOLDER="/home/datasets"
SCAN_LIST="experiments/alexa/alexatop1m.csv"
BEGIN_INDEX=1
END_INDEX=1000

FOLDER="$DATASET_FOLDER/$(date --iso-8601)-alexa-$END_INDEX"
echo "Creating dataset folder $FOLDER"
mkdir -p "$FOLDER"

scan_domain(){
    INDEX=$1
    DOMAIN=$2
    if (( INDEX >= BEGIN_INDEX )) && (( INDEX <= END_INDEX )); then
        SUMMARY="$FOLDER/Summary.md"
        echo "Executing scan for #$INDEX: $DOMAIN";
        printf -v INDEX "%06d" "$INDEX"
        ./start.sh --host "$DOMAIN" --tag "$INDEX$DOMAIN" --tlsattacker --datasetfolder "$FOLDER" --clientarguments "--repetitions $REPETITIONS --timeout 3000 --skip --noskip"

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

N=4
open_sem $N
while IFS=, read -r INDEX DOMAIN; do
    run_with_lock scan_domain "$INDEX" "$DOMAIN"
done < $SCAN_LIST

wait

