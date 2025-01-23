#!/bin/bash

#Jonatan's loading script. This is not gospel, just what worked for me



DIR=$(pwd)

LOG_FILE_RUN=$DIR/out_load #output just from this run
LOG_FILE_MAIN=/var/log/nfp-sdk6-rte.log #log containing this and all previous runs

FIRMWARE_FILE=$DIR/output/firmware.nffw
DESIGN_FILE=$DIR/output/pif_design.json
CONFIG_FILE=$DIR/config.p4cfg



#restart services
echo -n "Stopping services..."
sudo systemctl stop nfp-sdk6-rte
sudo systemctl stop nfp-hwdbg-srv
echo "done"

#unload old firmware
echo -n "Unloading old firmware..."
sudo /opt/netronome/bin/nfp-nffw unload
echo "done"

#kill old server
echo -n "Killing old server..."
sudo pkill pif_rte
echo "done"


#load new firmware and rules to card
echo "Loading new firmware from file"
pushd /opt/nfp_pif/bin/ > /dev/null
	sudo ./pif_rte -n 0 -p 20206 -I -z -s /opt/nfp_pif/scripts/pif_ctl_nfd.sh -f $FIRMWARE_FILE -d $DESIGN_FILE -c $CONFIG_FILE --log_file $LOG_FILE_MAIN > $LOG_FILE_RUN &
popd > /dev/null

echo piping load output to file $LOG_FILE_RUN
