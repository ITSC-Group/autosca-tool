#! /bin/bash
PARALLEL_THREADS=$(grep -c ^processor /proc/cpuinfo)
CROSSVALIDATION_TECHNIQUE="auto"
CROSSVALIDATION_ITERATIONS=30
HYPERPARAMETER_ITERATIONS=10

set -e

usage()
{
    echo 'usage example: ./analyze_dataset.sh --folder /home/datasets/2021-12-20-apollolvdamnvulnerableopenssl-server'
}

while [ "$1" != "" ]; do
    case $1 in
        --folder )              shift
                                FOLDER=$1
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

echo "Analyzing dataset folder $FOLDER"

CONFIG="$FOLDER/config.md"

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
pipenv run python3 classification_model/pvalues_calculation.py --folder="$FOLDER" 2>&1 | tee "$FOLDER/Report Generation $1.log"

echo "Plotting the machine learning results"
pipenv run python3 classification_model/plot_results.py --folder="$FOLDER" 2>&1 | tee "$FOLDER/Classification Model Plotting $1.log"
echo "Finished plotting"
echo " " >> "$CONFIG"

echo "Experiment run finished" >> "$CONFIG"
echo "Experiment run finished"