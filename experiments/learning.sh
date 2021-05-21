#! /bin/bash
FOLDERS=(
"2020-12-30-bearssl05server"
"2020-12-30-bouncycastletls158server"
"2020-12-30-damnvulnerableopensslserverfast"
"2020-12-30-damnvulnerableopensslserverfull"
"2020-12-30-jssetlsjre90412bc159server"
"2021-01-05-matrixsslserver430"
"2020-12-30-matrixssl340server"
"2020-12-30-mbedtls2130server"
"2020-12-30-openssl111eserver"
"2021-01-06-openssl097aserver"
)
DATASET_FOLDER="/home/datasets"
CROSSVALIDATION_ITERATIONS=30
HYPERPARAMETER_ITERATIONS=10
cd "./classification_model" || exit 1

for FOLDER in "${FOLDERS[@]}"
do
    echo "Plotting $FOLDER"
	  #pipenv run python3 train_models.py --folder="$DATASET_FOLDER/$FOLDER" --cv_iterations=$CROSSVALIDATION_ITERATIONS --iterations=$HYPERPARAMETER_ITERATIONS | tee "$DATASET_FOLDER/$FOLDER/Classification Model Training.log" &
	  pipenv run python3 plot_results.py --folder="$DATASET_FOLDER/$FOLDER" --cv_iterations=$CROSSVALIDATION_ITERATIONS | tee "$DATASET_FOLDER/$FOLDER/Classification Model Plotting.log" &
done
wait
