#! /bin/bash
TOOL_FOLDER=$(pwd)
# All python scripts use Pipenv
pip3 install --user pipenv
echo "After installing pipenv, you may need to restart your terminal"
# source ~/.profile

echo "Building TLS attacker client"
cd "$TOOL_FOLDER/tls_attacker_client" || exit
mvn clean install
echo "Installing feature extraction dependencies"
cd "$TOOL_FOLDER/feature_extraction" || exit
pipenv install
echo "Installing classification model dependencies"
cd "$TOOL_FOLDER/classification_model" || exit
pipenv install

