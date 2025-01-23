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

#include <core.p4>
#include <v1model.p4>
//#include <psa.p4>
//#include <p4d2model.p4>
#include "config.h"

const bit<16> TYPE_IPV4 = 0x800; //etherType for ipv4
const bit<16> TYPE_IPV6 = 0x86DD; //etherType for ipv6
const bit<16> TYPE_LLDP = 0x88CC;

const bit<8> TYPE_TCP = 0x06;
const bit<8> TYPE_UDP = 0x11;

typedef bit<16> egressSpec_t;
typedef bit<48> macAddr_t;
typedef bit<32> ip4Addr_t;


#define PREPEND_ENABLE 0 //enable hack/fix for initial 4 bytes being stripped in egress



extern void neuralNetworkEval(); //let ANN decide if it should evaluate the flow

extern void registerPacket(); //let the c plugin register data about the packet and flow


#if PREPEND_ENABLE

	header prepend_t
	{
		bit<32> fill;
	}

#endif


header ethernet_t
{
	macAddr_t dstAddr;
	macAddr_t srcAddr;
	bit<16> etherType;
}

header ipv4_t
{
	bit<4> version;
	bit<4> ihl;
	bit<8> diffserv;
	bit<16> totalLen;
	bit<16> identification;
	bit<3> flags;
	bit<13> fragOffset;
	bit<8> ttl;
	bit<8> protocol;
	bit<16> hdrChecksum;
	ip4Addr_t srcAddr;
	ip4Addr_t dstAddr;
}

header ipv6_t
{
	bit<4> version;
	bit<8> trafficClass;
	bit<20> flowLabel;
	bit<16> payloadLength;
	bit<8> nextHeader;
	bit<8> hopLimit;
	bit<128> srcAddr;
	bit<128> dstAddr;
}


header tcp_t
{
	bit<16> srcPort;
	bit<16> dstPort;
	bit<32> secNum;
	bit<32> ackNum;
	bit<4> dataOffset;
	bit<3> reserved;
	bit<3> ecn;
	bit<1> urg;
	bit<1> ack;
	bit<1> push;
	bit<1> rst;
	bit<1> syn;
	bit<1> fin;
	bit<16> cwndSize;
	bit<16> checksum;
	bit<16> urgentPointer;
}

header tcpOptions_t
{
	bit<32> options;
}

header payloadHead_t
{
	bit<32> data; //128 = 16*8
}



header udp_t
{
	bit<16> srcPort;
	bit<16> dstPort;
	bit<16> len;
	bit<16> checksum;
}

//what features should be recorded about the traffic flow
//@metadata @name("enabledFeatures_t)
struct enabledFeatures_t
{
	bit<1> ENABLE_FLOW_RECORDING; //required for everything. disable to just forward flow without recording it
	bit<1> ENABLE_DIRECTIONALITY; //required for all '*_forward' or '*_backward' features
	bit<1> ENABLE_TIME; //required for all time based features

	//implemented
	bit<1> packetCount; //This is recommended to keep active for every flow
	bit<1> packetCount_forward;
	bit<1> packetCount_backward;
	bit<1> totalFlowData;
	bit<1> totalFlowData_forward;
	bit<1> totalFlowData_backward;
	bit<1> tproto; //what transport protocol? tcp/udp?
	bit<1> maxPktSize; //size of largest packet
	bit<1> maxPktSize_forward;
	bit<1> maxPktSize_backward;
	bit<1> minPktSize; //size of smallest packet
	bit<1> minPktSize_forward;
	bit<1> minPktSize_backward;
	bit<1> aproto; //what application protocol? ssh/http/ftp/etc (add more application protocols!) 
	bit<1> enableTimer; //required for all time-based features
	bit<1> firstPktTime; //time first pkt was processed
	bit<1> lastPktTime; //time last pkt was processed (required for variance)
	
	bit<1> duration;
	bit<1> handshakeTimes;
	bit<1> isn_forward;
	bit<1> isn_backward;
	bit<1> tcpWin_forward;
	bit<1> tcpWin_backward;
	bit<1> ttl_forward;
	bit<1> ttl_backward;
	bit<1> retrans_forward;
	bit<1> retrans_backward;

}


struct metadata
{
	bit<4> tcpOptionsCounter;
	bit<16> payloadSize; //size of payload (in bytes)
	
	bit<11> padding;
	bit<1> doNeuralNetwork; //1 if this packet is the clone, which is just there to be analyzed
	enabledFeatures_t enabledFeatures;
}


struct headers
{
	#if PREPEND_ENABLE
		prepend_t prepend;
	#endif
	
	ethernet_t ethernet;
	ipv4_t ipv4;
	ipv6_t ipv6;

	//max 10 32-bit words in tcp options
	tcp_t tcp;
	tcpOptions_t tcpOpt0;
	tcpOptions_t tcpOpt1;
	tcpOptions_t tcpOpt2;
	tcpOptions_t tcpOpt3;
	tcpOptions_t tcpOpt4;
	tcpOptions_t tcpOpt5;
	tcpOptions_t tcpOpt6;
	tcpOptions_t tcpOpt7;
	tcpOptions_t tcpOpt8;
	tcpOptions_t tcpOpt9;

	udp_t udp;

	payloadHead_t payloadHead;
}




parser MyParser(packet_in packet, out headers hdr, inout metadata meta, inout standard_metadata_t standard_metadata)
{
	state start
	{
		//transition accept; //skip parsing headers

		transition parse_ethernet;
	}

	state parse_ethernet
	{
		packet.extract(hdr.ethernet);

		
		transition select(hdr.ethernet.etherType)
		{
			TYPE_IPV4: parse_ipv4; //if etherType ipv4 -> parse ipv4
			TYPE_IPV6: parse_ipv6;
			default: accept; //else -> just accept packet
		}
		
	}

	state parse_ipv4
	{
		packet.extract(hdr.ipv4);
 
		meta.payloadSize = hdr.ipv4.totalLen - 20;
		
		transition select(hdr.ipv4.protocol)
		{
			TYPE_TCP: parse_tcp;
			TYPE_UDP: parse_udp;
			default: accept;
		}

	}

	state parse_ipv6
	{
		packet.extract(hdr.ipv6);

		meta.payloadSize = hdr.ipv6.payloadLength;

		transition select(hdr.ipv6.nextHeader)
		{
			TYPE_TCP: parse_tcp;
			TYPE_UDP: parse_udp;
			default: accept;
		}
	}


	state parse_tcp
	{
		packet.extract(hdr.tcp);

		meta.tcpOptionsCounter = hdr.tcp.dataOffset;

		meta.payloadSize = meta.payloadSize - 20;
		
		transition select(meta.tcpOptionsCounter)
		{
			0x5: precheck_payloadHead; //if no tcp options
			default: parse_tcpOpt0; //should handle tcp options
		}

	   //transition accept;
	}

	state parse_tcpOpt0
	{   
		packet.extract(hdr.tcpOpt0);
		meta.payloadSize = meta.payloadSize - 4;
		transition select(meta.tcpOptionsCounter)
		{
			0x6: precheck_payloadHead; //if no more options
			default: parse_tcpOpt1; //parse more options
		}
	}
	state parse_tcpOpt1
	{   
		packet.extract(hdr.tcpOpt1);
		meta.payloadSize = meta.payloadSize - 4;
		transition select(meta.tcpOptionsCounter)
		{
			0x7: precheck_payloadHead; //if no more options
			default: parse_tcpOpt2; //parse more options
		}
	}
	state parse_tcpOpt2
	{   
		packet.extract(hdr.tcpOpt2);
		meta.payloadSize = meta.payloadSize - 4;
		transition select(meta.tcpOptionsCounter)
		{
			0x8: precheck_payloadHead; //if no tcp more options
			default: parse_tcpOpt3; //parse more options
		}
	}
	state parse_tcpOpt3
	{   
		packet.extract(hdr.tcpOpt3);
		meta.payloadSize = meta.payloadSize - 4;
		transition select(meta.tcpOptionsCounter)
		{
			0x9: precheck_payloadHead; //if no tcp more options
			default: parse_tcpOpt4; //parse more options
		}
	}
	state parse_tcpOpt4
	{   
		packet.extract(hdr.tcpOpt4);
		meta.payloadSize = meta.payloadSize - 4;
		transition select(meta.tcpOptionsCounter)
		{
			0xa: precheck_payloadHead; //if no tcp more options
			default: parse_tcpOpt5; //parse more options
		}
	}
	state parse_tcpOpt5
	{   
		packet.extract(hdr.tcpOpt5);
		meta.payloadSize = meta.payloadSize - 4;
		transition select(meta.tcpOptionsCounter)
		{
			0xb: precheck_payloadHead; //if no tcp more options
			default: parse_tcpOpt6; //parse more options
		}
	}
	state parse_tcpOpt6
	{   
		packet.extract(hdr.tcpOpt6);
		meta.payloadSize = meta.payloadSize - 4;
		transition select(meta.tcpOptionsCounter)
		{
			0xc: precheck_payloadHead; //if no tcp more options
			default: parse_tcpOpt7; //parse more options
		}
	}
	state parse_tcpOpt7
	{   
		packet.extract(hdr.tcpOpt7);
		meta.payloadSize = meta.payloadSize - 4;
		transition select(meta.tcpOptionsCounter)
		{
			0xd: precheck_payloadHead; //if no tcp more options
			default: parse_tcpOpt8; //parse more options
		}
	}
	state parse_tcpOpt8
	{   
		packet.extract(hdr.tcpOpt8);
		meta.payloadSize = meta.payloadSize - 4;
		transition select(meta.tcpOptionsCounter)
		{
			0xe: precheck_payloadHead; //if no tcp more options
			default: parse_tcpOpt9; //parse more options
		}
	}
	state parse_tcpOpt9
	{   
		packet.extract(hdr.tcpOpt9);
		meta.payloadSize = meta.payloadSize - 4;
		transition precheck_payloadHead; //can't be more than 10 options, so payload is next
	}


	state parse_udp
	{
		packet.extract(hdr.udp);

		meta.payloadSize = meta.payloadSize - 8;

		transition precheck_payloadHead;
	}

	//checking if there is any payload to parse
	state precheck_payloadHead
	{
		transition accept; //don't parse payload
		/*
		//Check size of payload
		transition select(meta.payloadSize)
		{
			0: accept; //payload empty
			default: parse_payloadHead;
		}
		*/
	}

	state parse_payloadHead
	{
		packet.extract(hdr.payloadHead);
		transition accept;
	}

}


control MyVerifyChecksum(inout headers hdr, inout metadata meta)
{
	apply
	{

	}
}

control MyIngress(inout headers hdr, inout metadata meta, inout standard_metadata_t standard_metadata)
{


		action packetForward(egressSpec_t port)
		{
			standard_metadata.egress_spec = port;
		}

		action packetDrop()
		{
			mark_to_drop();
		}

		
		action setRecordedFeatures(
			bit<1> ENABLE_FLOW_RECORDING,
			bit<1> ENABLE_DIRECTIONALITY,
			bit<1> ENABLE_TIME,
			bit<1> packetCount, 
			bit<1> packetCount_forward, 
			bit<1> packetCount_backward, 
			bit<1> totalFlowData,
			bit<1> totalFlowData_forward,
			bit<1> totalFlowData_backward,
			bit<1> tproto,
			bit<1> maxPktSize,
			bit<1> maxPktSize_forward,
			bit<1> maxPktSize_backward,
			bit<1> minPktSize,
			bit<1> minPktSize_forward,
			bit<1> minPktSize_backward,
			bit<1> aproto,
			bit<1> firstPktTime,
			bit<1> lastPktTime,

			bit<1> duration,
			bit<1> handshakeTimes,
			bit<1> isn_forward,
			bit<1> isn_backward,
			bit<1> tcpWin_forward,
			bit<1> tcpWin_backward,
			bit<1> ttl_forward,
			bit<1> ttl_backward,
			bit<1> retrans_forward,
			bit<1> retrans_backward
			)
		{


			#if FLOW_RECORDING
			
				//Set flags for enabled features
				meta.enabledFeatures.ENABLE_FLOW_RECORDING = ENABLE_FLOW_RECORDING;
				meta.enabledFeatures.ENABLE_DIRECTIONALITY = ENABLE_DIRECTIONALITY;
				meta.enabledFeatures.ENABLE_TIME = ENABLE_TIME;
				meta.enabledFeatures.packetCount = packetCount;
				meta.enabledFeatures.packetCount_forward = packetCount_forward;
				meta.enabledFeatures.packetCount_backward = packetCount_backward;
				meta.enabledFeatures.totalFlowData = totalFlowData;
				meta.enabledFeatures.totalFlowData_forward = totalFlowData_forward;
				meta.enabledFeatures.totalFlowData_backward = totalFlowData_backward;
				meta.enabledFeatures.tproto = tproto;
				meta.enabledFeatures.maxPktSize = maxPktSize;
				meta.enabledFeatures.maxPktSize_forward = maxPktSize_forward;
				meta.enabledFeatures.maxPktSize_backward = maxPktSize_backward;
				meta.enabledFeatures.minPktSize = minPktSize;
				meta.enabledFeatures.minPktSize_forward = minPktSize_forward;
				meta.enabledFeatures.minPktSize_backward = minPktSize_backward;
				meta.enabledFeatures.aproto = aproto;
				meta.enabledFeatures.firstPktTime = firstPktTime;
				meta.enabledFeatures.lastPktTime = lastPktTime;

				meta.enabledFeatures.duration = duration;
				meta.enabledFeatures.handshakeTimes = handshakeTimes;
				meta.enabledFeatures.isn_forward = isn_forward;
				meta.enabledFeatures.isn_backward = isn_backward;
				meta.enabledFeatures.tcpWin_forward = tcpWin_forward;
				meta.enabledFeatures.tcpWin_backward = tcpWin_backward;
				meta.enabledFeatures.ttl_forward = ttl_forward;
				meta.enabledFeatures.ttl_backward = ttl_backward;
				meta.enabledFeatures.retrans_forward = retrans_forward;
				meta.enabledFeatures.retrans_backward = retrans_backward;
				
			

				//Store ingress timestamp to metadata for pkt
				//meta.ingress_timestamp = intrinsic_metadata.ingress_global_timestamp;

				//meta.ingress_timestamp = standard_metadata.ingress_global_timestamp;
			

				registerPacket();

				/*
				#if ANN_EVAL && ANN_IN_INGRESS
					//Do neural network magic last thing in ingress
					neuralNetworkEval();
				#endif
				*/

			#endif
	
		}
		

		table routetable
		{
			key = { 
				standard_metadata.ingress_port: exact;
				meta.enabledFeatures.packetCount: exact;
				hdr.ipv4.dstAddr: exact;
				hdr.ipv4.srcAddr: exact;
			}

			actions = {
				packetForward;
				packetDrop;
			}
		}
		
		
		table featureCollection
		{
			key = {
				hdr.tcp.dstPort: exact;
				hdr.tcp.srcPort: exact;
				hdr.udp.dstPort: exact;
				hdr.udp.srcPort: exact;
				hdr.ipv4.dstAddr: exact;
				hdr.ipv4.srcAddr: exact;
			}

			actions = {
				setRecordedFeatures;
			}
		}
		


		apply
		{
			#if PREPEND_ENABLE
				hdr.prepend.setValid();
				hdr.prepend.fill = 0xffffffff;
			#endif
			
			

			routetable.apply();

			featureCollection.apply();
		}

	
}


control MyEgress(inout headers hdr, inout metadata meta, inout standard_metadata_t standard_metadata)
{
	apply
	{

	#if ANN_EVAL
		
		#if CLONE_FOR_ANN_EVAL //If a clone should be evaluated, after the original packet has been emitted

			if( meta.doNeuralNetwork == 1)
			{
				//Cloned pkt goes here

				//required, or the card won't clone packets after a while. (is this because of my static session_id?)
				meta.doNeuralNetwork = 0; 

				//this flow for this packet should just be analyzed by the neural network, then dropped
				neuralNetworkEval();

				//this is the clone, so just drop it
				mark_to_drop(); 
			}
			else
			{
				//Original pkt goes here
				meta.doNeuralNetwork = 1;

				//clone this packet to egress, then just transmit this packet (keeps the latency down)
				clone3(CloneType.E2E, 1, { meta.doNeuralNetwork });
			}

		#else //just evaluate packets directly in pipeline

			neuralNetworkEval();

		#endif

	#endif

	}
}

control MyComputeChecksum(inout headers hdr, inout metadata meta)
{
	apply
	{

	}
}

control MyDeparser(packet_out packet, in headers hdr)
{
	apply
	{
		#if PREPEND_ENABLE
			packet.emit(hdr.prepend); //send 4 prepend bytes (until I figure out how to disable metadata option)
		#endif

		//packet.emit(hdr.enabledFeatures); //just here temporarily for p4 to generate some functions

		packet.emit(hdr.ethernet);
		packet.emit(hdr.ipv4);
		packet.emit(hdr.ipv6);

		packet.emit(hdr.tcp);
		packet.emit(hdr.tcpOpt0);
		packet.emit(hdr.tcpOpt1);
		packet.emit(hdr.tcpOpt2);
		packet.emit(hdr.tcpOpt3);
		packet.emit(hdr.tcpOpt4);
		packet.emit(hdr.tcpOpt5);
		packet.emit(hdr.tcpOpt6);
		packet.emit(hdr.tcpOpt7);
		packet.emit(hdr.tcpOpt8);
		packet.emit(hdr.tcpOpt9);
	 
		packet.emit(hdr.udp);

		packet.emit(hdr.payloadHead);
	}
}

V1Switch(
	MyParser(), 
	MyVerifyChecksum(), 
	MyIngress(), 
	MyEgress(), 
	MyComputeChecksum(), 
	MyDeparser()
)main;


















