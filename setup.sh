#! /bin/bash
TOOL_FOLDER=$(pwd)
# All python scripts use Pipenv
pip3 install --user pipenv
echo "After installing pipenv, you may need to restart your terminal"
# source ~/.profile
# git gets really funky with the executable flag on Mac vs Linux machines
git config core.filemode false

git submodule update --recursive --remote --init

echo "Installing scriptable client dependencies" 
cd "$TOOL_FOLDER/scriptable_client" || exit
pipenv install
git config core.filemode false
echo "Installing feature extraction dependencies"
cd "$TOOL_FOLDER/feature_extraction" || exit
pipenv install
git config core.filemode false
echo "Installing classification model dependencies"
cd "$TOOL_FOLDER/classification_model" || exit
pipenv install
# pipenv run python3 setup.py install

echo "Building TLS attacker client"
cd "$TOOL_FOLDER/tls_attacker_client" || exit
mvn clean install

