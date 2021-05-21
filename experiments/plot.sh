#! /bin/bash
FOLDERS=(
"2020-12-07-damnvulnerableopensslserverfast"
"2020-12-07-damnvulnerableopensslserverfull"
)
CROSSVALIDATION_ITERATIONS=30
cd "../classification_model" || exit
for FOLDER in "${FOLDERS[@]}"
do
    pipenv run python3 scripts/plot_results.py --folder="../datasets/$FOLDER" --cv_iterations=$CROSSVALIDATION_ITERATIONS
done
