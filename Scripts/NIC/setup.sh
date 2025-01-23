#!/bin/bash

#unload firmware
echo "Unloading old firmware..."
sudo /opt/netronome/bin/nfp-nffw unload

#kill server
echo "Killing old server..."
sudo pkill pif_rte

#stop services
echo "Stopping services..."
sudo systemctl stop nfp-sdk6-rte
sudo systemctl stop nfp-hwdbg-srv

#reload kernel modules
echo "Restarting kernel modules..."
sudo depmod
sudo modprobe -r nfp #remove nfp kernel module
sudo modprobe nfp
#sudo modprobe nfp nfp_dev_cpp=1
#sudo modprobe nfp nfp_dev_cpp=1 nfp_pf_netdev=1 #load with debugging support
sudo modprobe devlink

#restart services
echo "Starting services..."
sudo systemctl restart nfp-sdk6-rte
sudo systemctl restart nfp-hwdbg-srv
