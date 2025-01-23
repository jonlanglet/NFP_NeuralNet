#!/bin/bash

echo "Adding environmental variable SDK_DIR..."
export SDK_DIR=/opt/netronome/

#this symlink is required for compilation
echo "Adding essential symlink..."
sudo ln -s /opt/netronome/p4/bin/p4c-bm2-ss /opt/netronome/p4/libexec/

