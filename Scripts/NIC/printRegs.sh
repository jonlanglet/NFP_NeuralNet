#!/bin/bash

NFDIR=/opt/netronome/bin

rtsym()
{
	echo "$1:"
	sudo $NFDIR/nfp-rtsym $1
}


regs=$(sudo $NFDIR/nfp-rtsym -L | awk '{ print $1; }')


#for reg in $regs
#do
#	rtsym $reg
#done

rtsym _pif_counter_RX
rtsym _pif_counter_TX
rtsym _pif_counter_RX_NFD
rtsym _pif_counter_TX_NFD
rtsym _pif_counter_DROP_NFD_NO_CREDITS
rtsym _pif_counter_FLOWCACHE_HIT
rtsym _pif_counter_FLOWCACHE_MISS
rtsym _pif_counter_DROP
rtsym _pif_counter_DROP_META

#whilea#
#rtsym $regs


#rtsym _pif_counter_RX

#rtsym _pif_counter_TX
#rtsym _pif_counter_TX_NFD
#rtsym _pif_counter_TX_WQ

#rtsym _pif_counter_ERROR_DEPARSE
#rtsym _pif_counter_ERROR_NFDMETA
#rtsym _pif_counter_ERROR_PARREP_CUSTOM
#rtsym _pif_counter_ERROR_PARREP
#rtsym _pif_counter_ERROR_EGRESS
#rtsym _pif_counter_ERROR_INGRESS
#rtsym _pif_counter_ERROR_FLOWCACHE
#rtsym _pif_counter_ERROR_PKTMETA

#rtsym _pif_counter_DROP
#rtsym _pif_counter_DROP_MCAST_PORT_OUT_BOUNDS
#rtsym _pif_counter_DROP_META
#rtsym _pif_counter_DROP_NFD_NO_CREDITS
#rtsym _pif_counter_DROP_INVALID_PKT_MOD_SCRIPT
#rtsym _pif_counter_DROP_PORT_PAUSED

#rtsym _pif_counter_FLOWCACHE_MISS
#rtsym _pif_counter_FLOWCACHE_HIT
