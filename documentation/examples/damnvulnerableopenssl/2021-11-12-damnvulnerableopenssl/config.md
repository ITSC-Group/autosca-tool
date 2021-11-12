# Experiment damnvulnerableopenssl
## Date
Fri Nov 12 14:28:03 CET 2021
## Host
machina
## Command Line Parameters
--name apollolv/damnvulnerableopenssl-server --tag damnvulnerableopenssl --docker --threads 1 --tlsattacker --port 44451 --datasetfolder /home/datasets --clientarguments --repetitions 500 --noskip
## Output Folder
/home/datasets/2021-11-12-damnvulnerableopenssl
## Prototype Version
git commit 456ef37
on branch extension

# Dataset generation
## Docker Command
docker run -it --rm -d --name=damnvulnerableopenssl -p 44451:4433  apollolv/damnvulnerableopenssl-server 
## Server Hostname/IP and Port
localhost:44451
## Capturing network traffic
From and to 172.17.0.2
On interface docker0
## Client Command
java -jar apps/ML-BleichenbacherGenerator.jar -connect localhost:44451 --folder "/home/datasets/2021-11-12-damnvulnerableopenssl" --repetitions 500 --noskip 2>&1 | tee "/home/datasets/2021-11-12-damnvulnerableopenssl/TLS Attacker.log"
## Execution Time
9 seconds
 
# Feature Extraction
## Execution Time
10 seconds
 
# Machine Learning
## Parameters
Using mccv crossvalidation technique
Doing 10 crossvalidation iterations
Doing 10 hyperparameter optimization iterations
## Execution Time
896 seconds
 
Experiment run finished
