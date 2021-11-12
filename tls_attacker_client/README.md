# ML-Bleichenbacher Generator
This project randomly creates TLS handshakes with a target while sending bleichenbacher vectors.
It prints the client random used and the fingerprint.

# Building
To build it you need TLS-Attacker version 3.6.0 (currently in the TLS-Attacker-Development repository).
First clone TLS-Attacker and build it with 

´´´mvn clean install´´´

then you can run this tool with the same command. In the apps folder should now be an executable jar which you can execute.

´´´java -jar apps/ML-BleichenbacherGenerator.jar´´´

This should print you the help command. An example use case should be something like this:

´´´java -jar apps/ML-BleichenbacherGenerator.jar -connect localhost:4433´´´


