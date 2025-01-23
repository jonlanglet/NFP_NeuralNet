/*
MIT License

Copyright (c) 2025 Jonatan Langlet

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/
#include <nfp.h>
#include <pif_plugin.h>
#include <mem_atomic.h> //heavily used to delegate modifications in EMEM without halting thread execution (and handling mutex)
#include "flowHandling.h"
#include "semaphore.h"

//#include "mac_time.h"
#include <nfp/me.h>

//volatile _declspec(emem export scope(global)) struct FlowMetadata flowMetadata[SIZE_FLOWTABLE];

_declspec(local_mem) time_t __flowHandling_currentTime__;




/*
	declare semaphores
*/
__declspec(ctm export aligned(64) mem addr40) int semaphore_updateTotalFlowData;


//volatile __declspec(mem addr40) void * addr



index_t calculateFlowIndex(EXTRACTED_HEADERS_T *headers)
{
	index_t index;
	int srcPort, dstPort;
	int srcIP, dstIP;

	PIF_PLUGIN_ipv4_T *ipv4Header = pif_plugin_hdr_readonly_get_ipv4(headers);
	//PIF_PLUGIN_ipv6_T *ipv6Header = pif_plugin_hdr_readonly_get_ipv6(headers);
	PIF_PLUGIN_tcp_T *tcpHeader = pif_plugin_hdr_readonly_get_tcp(headers);
	PIF_PLUGIN_udp_T *udpHeader = pif_plugin_hdr_readonly_get_udp(headers);
	
	srcIP = PIF_HEADER_GET_ipv4___srcAddr( ipv4Header );
	//srcIP += PIF_HEADER_GET_ipv6___srcAddr( ipv6Header ));

	dstIP = PIF_HEADER_GET_ipv4___dstAddr( ipv4Header );
	//dstIP += PIF_HEADER_GET_ipv6___dstAddr( ipv6Header ));

	srcPort = PIF_HEADER_GET_tcp___srcPort( tcpHeader );
	srcPort += PIF_HEADER_GET_udp___srcPort( udpHeader );
	 
	dstPort = PIF_HEADER_GET_tcp___dstPort( tcpHeader );
	dstPort += PIF_HEADER_GET_udp___dstPort( udpHeader );


	/*
		super simple hash function
		will generate same hash when src&dst are reversed, so flows count in both directions
		TODO: improve this to avoid intentional malicious collisions! (build-in crc?)
	*/
	index = srcIP + dstIP;
	index *= 31;
	index += srcPort + dstPort;
	index %= SIZE_FLOWTABLE;

	//Calculation of flow table index goes here
	return index;

}


/*
	Returns timestamp (incremented every 16:th cycle?)
*/
time_t getTime()
{
	return __flowHandling_currentTime__;
}

/*
	Converts input timestamp into milliseconds
	this conversion has been roughly measured by measuring two timestamps 100s apart
	(100 seconds was 5074521076)
*/
time_t getTime_ms(time_t timestamp)
{
	return timestamp/50745;
}
time_t getTime_us(time_t timestamp)
{
	return timestamp/51;
}


/*
	Terrible hack to get a timestamp for packets.
	TODO: retrieve MAC-timestamp which should be set in ingress!

	Updates global variable containing current time in nanoseconds
*/
void updateTime()
{
	/*

	//Handled by another process. Just trust this one
	__xread struct mac_time_state mac_time_state;
	struct mac_time_data time_data;
	uint64_t time;
	uint32_t time_ns;

	
	mac_time_fetch( &mac_time_state );

	//split it up into one row per operation. Mostly for debugging errors with 64-bits
	time = mac_time_state.mac_time_s;
	time *= 1000000000;
	time_ns = mac_time_state.mac_time_ns;
	time_ns %= 1000000000;
	time += time_ns;
	
	__flowHandling_currentTime__ = time;
	
	*/

	__flowHandling_currentTime__ = me_tsc_read();
}


/*
	special functions to read flow data because hi and lo 32-bit are swapped.
*/
uint64_t getTotalFlowData_forward(index_t flowIndex)
{
	//retrieve current raw value
	uint64_t result = flowMetadata[flowIndex].totalFlowData_forward;
	//swap first 32-bits with last 32-bits
	result = ( (result & 0x00000000ffffffff) << 32 ) | ( (result & 0xffffffff00000000) >> 32 ); 
	return result;
}
uint64_t getTotalFlowData_backward(index_t flowIndex)
{
	//retrieve current raw value
	uint64_t result = flowMetadata[flowIndex].totalFlowData_backward;
	//swap first 32-bits with last 32-bits
	result = ( (result & 0x00000000ffffffff) << 32 ) | ( (result & 0xffffffff00000000) >> 32 ); 
	return result;
}


void flow_updatePktCount(index_t flowIndex, EXTRACTED_HEADERS_T *headers)
{
	//pktCount++
	mem_incr32( (__mem40 void*)&flowMetadata[flowIndex].pktCount );
}

//only call this function if the packet is going forwards!
void flow_updatePktCount_forward(index_t flowIndex, EXTRACTED_HEADERS_T *headers)
{
	//pktCount_forward++
	mem_incr32( (__mem40 void*)&flowMetadata[flowIndex].pktCount_forward ); 
}

//only call this function if the packet is going backwards!
void flow_updatePktCount_backward(index_t flowIndex, EXTRACTED_HEADERS_T *headers)
{
	//pktCount_backward++
	mem_incr32( (__mem40 void*)&flowMetadata[flowIndex].pktCount_backward ); 
}


//TODO: add mutex for critical section! Also, fix atomic writes for improved performance in emem
//Max TCP packet-size is 2^16-1 bytes. mem_add64_imm can add values up to 16-bit to 64-bit variable. Use this? How to handle offset after add?
void flow_updateTotalFlowData(index_t flowIndex, EXTRACTED_HEADERS_T *headers)
{
	//get packet size
	pktsize_t pktSize = pif_plugin_meta_get__standard_metadata__packet_length(headers);
	//unsigned short pktSize = pif_plugin_meta_get__standard_metadata__packet_length(headers);

	//enter critical section
	//semaphore_down(&semaphore_updateTotalFlowData);

	//flowMetadata[flowIndex].totalFlowData += pktSize;
	mem_add64_imm( pktSize, (__mem40 void*)&flowMetadata[flowIndex].totalFlowData );


	//exit critical section
	//semaphore_up(&semaphore_updateTotalFlowData);



	//mem_add64_imm(pktSize, (__mem40 void*)&flowMetadata[flowIndex].totalFlowData);
	//mem_add64_imm(pktSize, (__declspec(aligned(4) addr40 mem) void*)&flowMetadata[flowIndex].totalFlowData);


}

//only call this function if the packet is going forwards!
void flow_updateTotalFlowData_forward(index_t flowIndex, EXTRACTED_HEADERS_T *headers)
{
	//totalFlowData += pktLength
	pktsize_t pktSize = pif_plugin_meta_get__standard_metadata__packet_length(headers);

	//TODO: add mutex for critical section
	//flowMetadata[flowIndex].totalFlowData_forward += pktSize;

	//mem_add64(__xwrite void *data, __mem40 void *addr, size_t size);
	mem_add64_imm( pktSize, (__mem40 void*)&flowMetadata[flowIndex].totalFlowData_forward );


	//totalFlowData_forward += pktLength
	//mem_add64_imm( pif_plugin_meta_get__standard_metadata__packet_length(headers), (__mem40 void*)&flowMetadata[flowIndex].totalFlowData_forward ); 
}

//only call this function if the packet is going backwards!
void flow_updateTotalFlowData_backward(index_t flowIndex, EXTRACTED_HEADERS_T *headers)
{
	//totalFlowData += pktLength
	pktsize_t pktSize = pif_plugin_meta_get__standard_metadata__packet_length(headers);

	//TODO: add mutex for critical section
	//flowMetadata[flowIndex].totalFlowData_backward += pktSize;
	mem_add64_imm( pktSize, (__mem40 void*)&flowMetadata[flowIndex].totalFlowData_backward );

	//totalFlowData_backward += pktLength
	//mem_add64_imm( pif_plugin_meta_get__standard_metadata__packet_length(headers), (__mem40 void*)&flowMetadata[flowIndex].totalFlowData_backward ); 
}

void flow_updateMaxPktSize(index_t flowIndex, EXTRACTED_HEADERS_T *headers)
{
	//TODO: Mutex-lock goes here!
	//Critical section because comparison before changing value, so mem_set isn't enough

	pktsize_t pktSize = pif_plugin_meta_get__standard_metadata__packet_length(headers);

	//MUTEX
	if( pktSize > flowMetadata[flowIndex].maxPktSize)
		flowMetadata[flowIndex].maxPktSize = pktSize;
}

void flow_updateMaxPktSize_forward(index_t flowIndex, EXTRACTED_HEADERS_T *headers)
{
	//TODO: Mutex-lock goes here!
	//Critical section because comparison before changing value, so mem_set isn't enough

	pktsize_t pktSize = pif_plugin_meta_get__standard_metadata__packet_length(headers);

	//MUTEX
	if( pktSize > flowMetadata[flowIndex].maxPktSize_forward)
		flowMetadata[flowIndex].maxPktSize_forward = pktSize;
}

void flow_updateMaxPktSize_backward(index_t flowIndex, EXTRACTED_HEADERS_T *headers)
{
	//TODO: Mutex-lock goes here!
	//Critical section because comparison before changing value, so mem_set isn't enough

	pktsize_t pktSize = pif_plugin_meta_get__standard_metadata__packet_length(headers);

	//MUTEX
	if( pktSize > flowMetadata[flowIndex].maxPktSize_backward)
		flowMetadata[flowIndex].maxPktSize_backward = pktSize;
}


void flow_updateMinPktSize(index_t flowIndex, EXTRACTED_HEADERS_T *headers)
{
	//TODO: Mutex-lock goes here!
	//Critical section because comparison before changing value, so mem_set isn't enough

	pktsize_t pktSize = pif_plugin_meta_get__standard_metadata__packet_length(headers);

	//MUTEX
	if( pktSize < flowMetadata[flowIndex].minPktSize)
		flowMetadata[flowIndex].minPktSize = pktSize;
}

void flow_updateMinPktSize_forward(index_t flowIndex, EXTRACTED_HEADERS_T *headers)
{
	//TODO: Mutex-lock goes here!
	//Critical section because comparison before changing value, so mem_set isn't enough

	pktsize_t pktSize = pif_plugin_meta_get__standard_metadata__packet_length(headers);

	//MUTEX
	if( pktSize < flowMetadata[flowIndex].minPktSize_forward)
		flowMetadata[flowIndex].minPktSize_forward = pktSize;
}

void flow_updateMinPktSize_backward(index_t flowIndex, EXTRACTED_HEADERS_T *headers)
{
	//TODO: Mutex-lock goes here!
	//Critical section because comparison before changing value, so mem_set isn't enough

	pktsize_t pktSize = pif_plugin_meta_get__standard_metadata__packet_length(headers);

	//MUTEX
	//smallest backwards packet has to check if this is the first, if so this is the smallest
	if( flowMetadata[flowIndex].minPktSize_backward==0 ||  pktSize < flowMetadata[flowIndex].minPktSize_backward)
		flowMetadata[flowIndex].minPktSize_backward = pktSize;
}

void flow_updateTimestamp_lastPkt(index_t flowIndex, EXTRACTED_HEADERS_T *headers)
{
	flowMetadata[flowIndex].lastTime = getTime();
}

void flow_updateRetranCount_forward(index_t flowIndex, EXTRACTED_HEADERS_T *headers)
{
	PIF_PLUGIN_tcp_T *tcpHeader = pif_plugin_hdr_readonly_get_tcp(headers);
	uint32_t thisSeqnum = PIF_HEADER_GET_tcp___secNum(tcpHeader);

	if( thisSeqnum <= flowMetadata[flowIndex].lastSeqnum_forward ) //if this is a retransmission
	{
		mem_incr32( (__mem40 void*)&flowMetadata[flowIndex].retrans_forward );
	}
	else
	{
		//update last seqnum with this one
		flowMetadata[flowIndex].lastSeqnum_forward = thisSeqnum;
	}
}
void flow_updateRetranCount_backward(index_t flowIndex, EXTRACTED_HEADERS_T *headers)
{
	PIF_PLUGIN_tcp_T *tcpHeader = pif_plugin_hdr_readonly_get_tcp(headers);
	uint32_t thisSeqnum = PIF_HEADER_GET_tcp___secNum(tcpHeader);

	if( thisSeqnum <= flowMetadata[flowIndex].lastSeqnum_backward ) //if this is a retransmission
	{
		mem_incr32( (__mem40 void*)&flowMetadata[flowIndex].retrans_backward );
	}
	else
	{
		//update last seqnum with this one
		flowMetadata[flowIndex].lastSeqnum_backward = thisSeqnum;
	}
}

void flow_updateLoad_forward(index_t flowIndex)
{
	uint32_t load;
	
	load = flowMetadata[flowIndex].totalFlowData_forward * 1000 / (getTime_ms(flowMetadata[flowIndex].flowDuration));

	flowMetadata[flowIndex].load_forward = load;
}
void flow_updateLoad_backward(index_t flowIndex)
{
	uint32_t load;
	
	load = flowMetadata[flowIndex].totalFlowData_backward * 1000 / (getTime_ms(flowMetadata[flowIndex].flowDuration));

	flowMetadata[flowIndex].load_backward = load;
}



void flow_setISN_forward(index_t flowIndex, EXTRACTED_HEADERS_T *headers)
{
	PIF_PLUGIN_tcp_T *tcpHeader = pif_plugin_hdr_readonly_get_tcp(headers);

	if( PIF_HEADER_GET_tcp___syn(tcpHeader) == 1 ) //if this is SYN pkt, read ISN
	{
		flowMetadata[flowIndex].ISN_forward = PIF_HEADER_GET_tcp___secNum(tcpHeader);
	}
}
void flow_setISN_backward(index_t flowIndex, EXTRACTED_HEADERS_T *headers)
{
	PIF_PLUGIN_tcp_T *tcpHeader = pif_plugin_hdr_readonly_get_tcp(headers);

	if( PIF_HEADER_GET_tcp___syn(tcpHeader) == 1 ) //if this is SYN pkt, read ISN
	{
		flowMetadata[flowIndex].ISN_backward = PIF_HEADER_GET_tcp___secNum(tcpHeader);
	}
}


void flow_updateTCPWin_forward(index_t flowIndex, EXTRACTED_HEADERS_T *headers)
{
	PIF_PLUGIN_tcp_T *tcpHeader = pif_plugin_hdr_readonly_get_tcp(headers);

	flowMetadata[flowIndex].tcpWindow_source = PIF_HEADER_GET_tcp___cwndSize(tcpHeader);
}
void flow_updateTCPWin_backward(index_t flowIndex, EXTRACTED_HEADERS_T *headers)
{
	PIF_PLUGIN_tcp_T *tcpHeader = pif_plugin_hdr_readonly_get_tcp(headers);

	flowMetadata[flowIndex].tcpWindow_dest = PIF_HEADER_GET_tcp___cwndSize(tcpHeader);
}

void flow_updateTTL_forward(index_t flowIndex, EXTRACTED_HEADERS_T *headers)
{
	PIF_PLUGIN_ipv4_T *ipv4Header = pif_plugin_hdr_readonly_get_ipv4(headers);
	flowMetadata[flowIndex].ttl_forward = PIF_HEADER_GET_ipv4___ttl(ipv4Header);
}
void flow_updateTTL_backward(index_t flowIndex, EXTRACTED_HEADERS_T *headers)
{
	PIF_PLUGIN_ipv4_T *ipv4Header = pif_plugin_hdr_readonly_get_ipv4(headers);
	flowMetadata[flowIndex].ttl_backward = PIF_HEADER_GET_ipv4___ttl(ipv4Header);
}

void flow_updateDuration(index_t flowIndex, EXTRACTED_HEADERS_T *headers)
{
	flowMetadata[flowIndex].flowDuration = getTime() - flowMetadata[flowIndex].initTime;
}

void flow_setHandshakeTimes(index_t flowIndex, EXTRACTED_HEADERS_T *headers)
{
	PIF_PLUGIN_tcp_T *tcpHeader = pif_plugin_hdr_readonly_get_tcp(headers);
	
	uint8_t syn, ack; //flags

	//only do this for first 3 packets. After that, handshake is done.
	if( flowMetadata[flowIndex].pktCount > 3) return;
	
	syn = PIF_HEADER_GET_tcp___syn( tcpHeader );
	ack = PIF_HEADER_GET_tcp___ack( tcpHeader );

	
	if( syn && !ack ) //if syn, do nothing. Its timestamp will be recorded as lastpkt
	{
		flowMetadata[flowIndex].synTime = getTime();

	}
	else if (syn && ack) //if synack
	{
		//calculate time between syn and synack
		flowMetadata[flowIndex].synackTime = getTime();
		flowMetadata[flowIndex].tcpSynSynackTime = getTime() - flowMetadata[flowIndex].synTime; 
	}
	else if (!syn && ack) //if end of handshake
	{
		//calculate time between synack and ack
		flowMetadata[flowIndex].tcpSynackAckTime = getTime() - flowMetadata[flowIndex].synackTime; 

		//calculate total time for handshake (first rtt)
		flowMetadata[flowIndex].tcpSetupRTT = flowMetadata[flowIndex].tcpSynSynackTime + flowMetadata[flowIndex].tcpSynackAckTime;
	}


}




/*
	Detection of application protocol is based only on destination port for first pkt
*/
void flow_setProto_A(index_t flowIndex, EXTRACTED_HEADERS_T *headers)
{
	uint16_t serverPort;
	PIF_PLUGIN_ipv4_T *ipv4Header = pif_plugin_hdr_readonly_get_ipv4(headers);

	serverPort = flowMetadata[flowIndex].serverPort;

	if( PIF_HEADER_GET_ipv4___protocol(ipv4Header) == 0x06 ) //if tcp
	{
		switch(serverPort)
		{
			case 80:
			case 443:
				flowMetadata[flowIndex].aProto_http = 1; //handle both http and https as same protocol
				break;

			case 22:
				flowMetadata[flowIndex].aProto_ssh = 1;
				break;

			case 21:
				flowMetadata[flowIndex].aProto_ftp = 1;
				break;
		}
	}
	else
	{
		
	}
}





void flow_initialize(index_t flowIndex, EXTRACTED_HEADERS_T *headers)
{
	__lmem struct pif_header_ingress__enabledFeatures *enabledFeatures;

	pktsize_t pktSize;


	//these are optimized away, so keep them for legibility
	PIF_PLUGIN_ipv4_T *ipv4Header = pif_plugin_hdr_readonly_get_ipv4(headers);
	PIF_PLUGIN_tcp_T *tcpHeader = pif_plugin_hdr_readonly_get_tcp(headers);
	PIF_PLUGIN_udp_T *udpHeader = pif_plugin_hdr_readonly_get_udp(headers);   
	 

	//handle ipv4 addresses
	flowMetadata[flowIndex].clientAddr = PIF_HEADER_GET_ipv4___srcAddr( ipv4Header );
	flowMetadata[flowIndex].serverAddr = PIF_HEADER_GET_ipv4___dstAddr( ipv4Header );

	//handle both tcp & udp clientport
	flowMetadata[flowIndex].clientPort = PIF_HEADER_GET_tcp___srcPort( tcpHeader );
	flowMetadata[flowIndex].clientPort += PIF_HEADER_GET_udp___srcPort( udpHeader );

	//handle both tcp & udp serverport
	flowMetadata[flowIndex].serverPort = PIF_HEADER_GET_tcp___dstPort( tcpHeader );
	flowMetadata[flowIndex].serverPort += PIF_HEADER_GET_udp___dstPort( udpHeader );

	//specify which direction is forwards, and which is backwards
	flowMetadata[flowIndex].clientLowerIP = (flowMetadata[flowIndex].clientAddr < flowMetadata[flowIndex].serverAddr);


	
	/*
		conditional features
	*/
	enabledFeatures = (__lmem struct pif_header_ingress__enabledFeatures *) (headers + PIF_PARREP_ingress__enabledFeatures_OFF_LW);
		
	pktSize = pif_plugin_meta_get__standard_metadata__packet_length(headers);

	//Smallest forward packet so far is the first. This way we don't have to handle that special case for every packet
	if(enabledFeatures->minPktSize)
		flowMetadata[flowIndex].minPktSize = pktSize;
	if(enabledFeatures->minPktSize_forward)
		flowMetadata[flowIndex].minPktSize_forward = pktSize;
	
	if(enabledFeatures->aproto)
		flow_setProto_A(flowIndex, headers);


	//what transport protocol?
	if(enabledFeatures -> tproto)
	{
		if( PIF_HEADER_GET_ipv4___protocol(ipv4Header) == 0x06 ) //if tcp
			flowMetadata[flowIndex].tproto_tcp = 1;

		else if( PIF_HEADER_GET_ipv4___protocol(ipv4Header) == 0x11 ) //if udp
			flowMetadata[flowIndex].tproto_udp = 1;
	}

	if(enabledFeatures->firstPktTime)
		flowMetadata[flowIndex].initTime = getTime();
}


/*
	Calculate and update flow metadata to prepare for ANN processing
*/
void prepareForANN(index_t flowIndex)
{
	//Calculate mean pkt sizes in both directions
	if(flowMetadata[flowIndex].pktCount_forward != 0)
		flowMetadata[flowIndex].meanPktSize_forward = getTotalFlowData_forward(flowIndex)/flowMetadata[flowIndex].pktCount_forward;
	if(flowMetadata[flowIndex].pktCount_backward != 0)
		flowMetadata[flowIndex].meanPktSize_backward = getTotalFlowData_backward(flowIndex)/flowMetadata[flowIndex].pktCount_backward;

	flow_updateLoad_forward(flowIndex);
	flow_updateLoad_backward(flowIndex);
}


/*
	record data about the packet, and the corresponding flow
*/
int pif_plugin_registerPacket(EXTRACTED_HEADERS_T *headers, ACTION_DATA_T *action_data)
{
	index_t flowIndex;
	int isForwardPacket;

	PIF_PLUGIN_ipv4_T *ipv4Header = pif_plugin_hdr_readonly_get_ipv4(headers);


	//this is how you access the metadata in micro-c
	__lmem struct pif_header_ingress__enabledFeatures *enabledFeatures;
	enabledFeatures = (__lmem struct pif_header_ingress__enabledFeatures *) (headers + PIF_PARREP_ingress__enabledFeatures_OFF_LW);
	//enabledFeatures->packetCount == 1 for example


	if( !enabledFeatures->ENABLE_FLOW_RECORDING ) return PIF_PLUGIN_RETURN_FORWARD; //if this flow shouldn't be recorded


	//get index for flow. based on ip and port for src&dst
	flowIndex = calculateFlowIndex(headers);

	//Update global variable containing current time
	if( enabledFeatures->ENABLE_TIME )
		updateTime();


	//initialize flow
	if(flowMetadata[flowIndex].pktCount == 0) //if this is the first packet in the flow
		flow_initialize(flowIndex, headers);



	/*
		Conditional feature collection
	*/

	//Update packet count
	if( enabledFeatures->packetCount )
		flow_updatePktCount(flowIndex, headers);

	//Update total flow data
	if( enabledFeatures->totalFlowData )
		flow_updateTotalFlowData(flowIndex, headers);

	
	//Update max packet size
	if(enabledFeatures->maxPktSize)
		flow_updateMaxPktSize(flowIndex, headers);

	//Update min packet size
	if(enabledFeatures->minPktSize)
		flow_updateMinPktSize(flowIndex, headers);

	//update total flow duration
	if(enabledFeatures->duration)
		flow_updateDuration(flowIndex, headers);

	//recording times during handshake. Just needs to run first 3 flow packets
	if(enabledFeatures->handshakeTimes)
		flow_setHandshakeTimes(flowIndex, headers);
	

	//Handle all direction-based features
	if( enabledFeatures->ENABLE_DIRECTIONALITY )
	{
		//find out if this packet is going forwards or backwards, based on values of IP addresses
		isForwardPacket = ( flowMetadata[flowIndex].clientLowerIP == (PIF_HEADER_GET_ipv4___srcAddr(ipv4Header) < PIF_HEADER_GET_ipv4___dstAddr(ipv4Header)) );

		if(isForwardPacket)
		{
			if( enabledFeatures->packetCount_forward )
				flow_updatePktCount_forward(flowIndex, headers);
		
			if( enabledFeatures->totalFlowData_forward )
				flow_updateTotalFlowData_forward(flowIndex, headers);

			if(enabledFeatures->maxPktSize_forward)
				flow_updateMaxPktSize_forward(flowIndex, headers);

			if(enabledFeatures->minPktSize_forward)
				flow_updateMinPktSize_forward(flowIndex, headers);

			//includes a check to see in SYN pkt
			if(enabledFeatures->isn_forward)
				flow_setISN_forward(flowIndex, headers);

			if(enabledFeatures->tcpWin_forward)
				flow_updateTCPWin_forward(flowIndex, headers);

			if(enabledFeatures->ttl_forward)
				flow_updateTTL_forward(flowIndex, headers);

			if(enabledFeatures->retrans_forward)
				flow_updateRetranCount_forward(flowIndex, headers);

		}
		else
		{
			if( enabledFeatures->packetCount_backward )
				flow_updatePktCount_backward(flowIndex, headers);
		
			if( enabledFeatures->totalFlowData_backward )
				flow_updateTotalFlowData_backward(flowIndex, headers);

			if(enabledFeatures->maxPktSize_backward)
				flow_updateMaxPktSize_backward(flowIndex, headers);

			if(enabledFeatures->minPktSize_backward)
				flow_updateMinPktSize_backward(flowIndex, headers);
			
			if(enabledFeatures->isn_backward)
				flow_setISN_backward(flowIndex, headers);//includes a check to see in SYN pkt

			if(enabledFeatures->tcpWin_backward)
				flow_updateTCPWin_backward(flowIndex, headers);

			if(enabledFeatures->ttl_backward)
				flow_updateTTL_backward(flowIndex, headers);

			if(enabledFeatures->retrans_backward)
				flow_updateRetranCount_backward(flowIndex, headers);

		}
	}

	

	//Update last pkt time (must run after all standard deviation calculations)
	//Do this last
	if(enabledFeatures->lastPktTime)
		flow_updateTimestamp_lastPkt(flowIndex, headers);


	return PIF_PLUGIN_RETURN_FORWARD;
}