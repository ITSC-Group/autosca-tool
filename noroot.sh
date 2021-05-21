#!/usr/bin/env bash

echo "This will let anyone who belongs to the 'pcap' group"
echo "execute 'tcpdump' and 'tc', instead of root/sudoers only."
echo "The user running this script MUST be a sudoer."

# Create new group "pcap"
sudo groupadd pcap
# Adds the current user to the pcap group
sudo usermod -a -G pcap $USER
# Let pcap own the tcpdump/tc binary
sudo chgrp pcap /usr/sbin/tcpdump
sudo chgrp pcap /usr/sbin/tc
# Allow non-root users to capture raw traffic with the tcpdump/tc binary
sudo setcap cap_net_raw,cap_net_admin=eip /usr/sbin/tcpdump
sudo setcap cap_net_raw,cap_net_admin=eip /usr/sbin/tc
# Link to the root tcpdump/tc binary in the non-root binaries folder
sudo ln -s /usr/sbin/tcpdump /usr/bin/tcpdump
sudo ln -s /usr/sbin/tc /usr/bin/tc

echo "Success"