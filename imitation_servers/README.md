# Imitation Servers

Servers imitating the ROBOT server behavior.
Due to export restrictions, the actual binary of the imitation servers can only be retrieved upon request from [achelos](https://www.achelos.de/en/tls-test-tool.html).
These can then be built into a single docker image using the dockerfile and the configuration subfolder contained in this folder:

```docker build -t imitation-server:8.0 .```