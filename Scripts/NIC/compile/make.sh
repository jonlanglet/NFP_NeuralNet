#!/bin/bash

P4_DIR=/opt/netronome/p4


wrongUsage()
{
	echo	
	echo "Usage:"
	echo " ./make.sh -14 main.p4 #makes main.p4 p4_14 style"
	echo " ./make.sh -16 main.p4 #makes main.p4 p4_16 style"
	echo
}


if [ $# -lt 2 ] ; then

	wrongUsage

elif [ $1 = "-16" ] ; then

	$P4_DIR/bin/nfp4build -e -o output/firmware.nffw -l beryllium --nfp4c_I $P4_DIR/include/16/p4include/ --nfp4c_p4_version 16 -4 ${@:2}

elif [ $1 = "-14" ] ; then

	$P4_DIR/bin/nfp4build -o output/firmware.nffw -l beryllium -4 ${@:2}

else
	wrongUsage
fi




