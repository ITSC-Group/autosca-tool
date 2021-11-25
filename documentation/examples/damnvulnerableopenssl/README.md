# DamnVulnerableOpenSSL Example
In this example experiment, we are using a public docker container as the server.
The container `apollolv/damnvulnerableopenssl-server` is running [DamnVulnerableOpenSSL](https://github.com/tls-attacker/DamnVulnerableOpenSSL), a version of OpenSSL modified to be vulnerable to Bleichenbacher side channel attacks.

We are using the following command to run the experiment:

```
/start.sh --name apollolv/damnvulnerableopenssl-server --tag "damnvulnerableopenssl" --docker --threads 1 --tlsattacker --port 44451 --datasetfolder /home/datasets --clientarguments "--repetitions 500 --noskip"
```

The results of this experiment are contained in an output folder like `/home/datasets/2021-11-12-damnvulnerableopenssl`.
You can find the results of this experiment in the [autosca-data repository](https://github.com/ITSC-Group/autosca-data) as well.
