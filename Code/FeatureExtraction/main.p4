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



extern void neuralNetworkEval();
extern void semaphore_lock(); //handles flow id's. Just set metadata to relative ID in flow
extern void semaphore_release();


//macros to handle mutex locks
//#define semFlowLock(semID) meta.semaphore_id = meta.flowIndex*SEM_LOCKS_PER_FLOW+semID; semaphore_lock() //lock semaphore "id" under flow
//#define semFlowRelease(semID) meta.semaphore_id = meta.flowIndex*SEM_LOCKS_PER_FLOW+semID; semaphore_release() //release last locked semaphore



enum HandshakeState
{
	initializing,
	handshakeSYN,
	handshakeSYNACK,
	handshakeACK,
	handshakeComplete
}



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


struct flowFeatures_t
{
	ip4Addr_t clientAddr;
	ip4Addr_t serverAddr;

	bit<1> clientLowerIP;

	bit<8> clientPort;
	bit<8> serverPort;

	bit<64> firstTime_forward;
	bit<64> firstTime_backward;

	bit<64> lastTime_total;
	bit<64> lastTime_forward;
	bit<64> lastTime_backward;

	bit<64> flowDuration_total;

	bit<32> pktCount_total;
	bit<32> pktCount_forward;
	bit<32> pktCount_backward;

	bit<64> dataVolume_total;
	bit<64> dataVolume_forward;
	bit<64> dataVolume_backward;

	bit<1> tproto_tcp;
	bit<1> tproto_udp;

	bit<32> maxPktSize_total;
	bit<32> maxPktSize_forward;
	bit<32> maxPktSize_backward;

	bit<32> minPktSize_total;
	bit<32> minPktSize_forward;
	bit<32> minPktSize_backward;

	bit<32> acknum_forward;
	bit<32> acknum_backward;

	bit<32> lastSeqnum_forward;
	bit<32> lastSeqnum_backward;

	bit<32> isn_forward;
	bit<32> isn_backward;

	bit<64> handshakeSynTime;
	bit<64> handshakeSynackTime;
	bit<64> handshakeAckTime;
	
	HandshakeState handshakeState; //stored the current state of the flow (used when analysing handshake)

	bit<16> tcpWindow_forward;
	bit<16> tcpWindow_backward;

	bit<8> ttl_forward;
	bit<8> ttl_backward;

	bit<32> lastANNTriggered;
	bit<8> lastANNOutput;
	bit<32> pktCount_egress;
	bit<32> pktCount_collectedFeatures;
}



struct intrinsic_metadata_t
{
	bit<64> ingress_global_timestamp; //ingress timestamp in microseconds
}   




struct metadata
{
	bit<4> tcpOptionsCounter;
	bit<16> payloadSize; //size of payload (in bytes)
	
	@name(".intrinsic_metadata") intrinsic_metadata_t intrinsic_metadata;

	bit<32> flowIndex;
	flowFeatures_t flowFeatures;

	bit<1> isForwardPacket;

	bit<1> collectFeatures;
	
	bit<1> performNNInference;

	bit<32> pktsSinceANNPerformed;

	bit<32> annTriggerInterval;

	bit<32> pktCount_annTrigger;

	bit<32> semaphore_id;

}


struct headers
{
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



/*
	List of features stored in registers
*/
register <bit<32>>(FLOW_METADATA_MAXNUM) clientAddr; //Source IP addr in first flow packet
register <bit<32>>(FLOW_METADATA_MAXNUM) serverAddr; //Destination IP addr in first flow packet
register <bit<16>>(FLOW_METADATA_MAXNUM) clientPort; //TCP/UDP source port in first flow packet
register <bit<16>>(FLOW_METADATA_MAXNUM) serverPort; //TCP/UDP destination port in first flow packet
register <bit<32>>(FLOW_METADATA_MAXNUM) pktCount_total; //Total recorded packets in flow
register <bit<32>>(FLOW_METADATA_MAXNUM) pktCount_forward; //Total recorded packets C->S
register <bit<32>>(FLOW_METADATA_MAXNUM) pktCount_backward; //Total recorded packests S->C
register <bit<64>>(FLOW_METADATA_MAXNUM) dataVolume_total; //Total data volume in flow
register <bit<64>>(FLOW_METADATA_MAXNUM) dataVolume_forward; //Total data volume from C->S
register <bit<64>>(FLOW_METADATA_MAXNUM) dataVolume_backward; //Total data volume from S->C
register <bit<1>>(FLOW_METADATA_MAXNUM) clientLowerIP; //If clientIP<serverIP. Used when determining directionality of packets
register <bit<1>>(FLOW_METADATA_MAXNUM) tproto_tcp; //If TCP
register <bit<1>>(FLOW_METADATA_MAXNUM) tproto_udp; //If UDP
register <bit<32>>(FLOW_METADATA_MAXNUM) maxPktSize_total; //Largest packet seen in flow
register <bit<32>>(FLOW_METADATA_MAXNUM) maxPktSize_forward; //Largest packet seen from C->S
register <bit<32>>(FLOW_METADATA_MAXNUM) maxPktSize_backward; //Largest packet seen from S->C
register <bit<32>>(FLOW_METADATA_MAXNUM) minPktSize_total; //Smallest packet seen in flow
register <bit<32>>(FLOW_METADATA_MAXNUM) minPktSize_forward; //Smallest packet seen from C->S
register <bit<32>>(FLOW_METADATA_MAXNUM) minPktSize_backward; //Smallest packet seen from S->C
register <bit<64>>(FLOW_METADATA_MAXNUM) firstTime_forward; //Timestamp for first packet in flow
register <bit<64>>(FLOW_METADATA_MAXNUM) firstTime_backward; //Timestamp for first packet from S->C
register <bit<64>>(FLOW_METADATA_MAXNUM) lastTime_total; //Timestamp of last packet in flow
register <bit<64>>(FLOW_METADATA_MAXNUM) lastTime_forward; //Timestamp of last packet from C->S
register <bit<64>>(FLOW_METADATA_MAXNUM) lastTime_backward; //Timestamp of last packet from S->C
register <bit<64>>(FLOW_METADATA_MAXNUM) flowDuration_total; //Total duration of traffic flow
register <bit<1>>(FLOW_METADATA_MAXNUM) aProto_http; //If HTTP
register <bit<1>>(FLOW_METADATA_MAXNUM) aProto_ssh; //If SSH
register <bit<1>>(FLOW_METADATA_MAXNUM) aProto_ftp; //If FTP
register <bit<1>>(FLOW_METADATA_MAXNUM) aProto_smtp; //If SMTP
register <bit<32>>(FLOW_METADATA_MAXNUM) acknum_forward; //Number of ACK packets C->S
register <bit<32>>(FLOW_METADATA_MAXNUM) acknum_backward; //Number of ACK packets S->C
register <bit<32>>(FLOW_METADATA_MAXNUM) lastSeqnum_forward; //TCP sequence number of last packet from C->S
register <bit<32>>(FLOW_METADATA_MAXNUM) lastSeqnum_backward; //TCP sequence number of last packet from S->C
register <bit<32>>(FLOW_METADATA_MAXNUM) isn_forward; //TCP sequence number of first packet
register <bit<32>>(FLOW_METADATA_MAXNUM) isn_backward; //TCP sequence number of first packet from S->C
register <bit<64>>(FLOW_METADATA_MAXNUM) handshakeSynTime; //Timestamp of SYN packet in TCP handshake
register <bit<64>>(FLOW_METADATA_MAXNUM) handshakeSynackTime; //Timestamp of SYNACK packet in TCP handshake
register <bit<64>>(FLOW_METADATA_MAXNUM) handshakeAckTime; //Timestamp of ACK packet in TCP handshake
register <HandshakeState>(FLOW_METADATA_MAXNUM) handshakeState; //The current state of the TCP handshake
register <bit<16>>(FLOW_METADATA_MAXNUM) tcpWindow_forward; //Advertised TCP congestion windows of last packet from C->S
register <bit<16>>(FLOW_METADATA_MAXNUM) tcpWindow_backward; //Advertised TCP congestion windows of last packet from S->C
register <bit<8>>(FLOW_METADATA_MAXNUM) ttl_forward; //IP TTL of last packet from C->S
register <bit<8>>(FLOW_METADATA_MAXNUM) ttl_backward; //IP TTL of last packet from S->C
//register <bit<32>>(FLOW_METADATA_MAXNUM) lastANNTriggered; //pktCount during previous ANN inference
register <bit<8>>(FLOW_METADATA_MAXNUM) lastANNOutput; //output neuron with highest activation last time ANN executed
//register <bit<32>>(FLOW_METADATA_MAXNUM) pktCount_egress; //total number of packets in flow which reached egress
register <bit<32>>(FLOW_METADATA_MAXNUM) pktCount_collectedFeatures; //total number of packets included in feature collection

register <bit<32>>(1) pktCount_annTrigger; //How many packets to include in feature collection before a freeze occurs (static, global)






parser MyParser(packet_in packet, out headers hdr, inout metadata meta, inout standard_metadata_t standard_metadata)
{
	state start
	{
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



	//Called during processing of first packet in each flow
	action initFlow()
	{
		clientAddr.write(meta.flowIndex, hdr.ipv4.srcAddr );
		serverAddr.write(meta.flowIndex, hdr.ipv4.dstAddr );

		//add tcpPort+udpPort to be compatible with both (one is always 0)
		clientPort.write(meta.flowIndex, hdr.tcp.srcPort + hdr.udp.srcPort);
		serverPort.write(meta.flowIndex, hdr.tcp.dstPort + hdr.udp.dstPort);

		//identify transport protocol by checking if a port has been parsed
		tproto_tcp.write(meta.flowIndex, (bit<1>)(hdr.tcp.dstPort!=0) );
		tproto_udp.write(meta.flowIndex, (bit<1>)(hdr.udp.dstPort!=0) );

		clientLowerIP.write(meta.flowIndex, (bit<1>)(hdr.ipv4.srcAddr<hdr.ipv4.dstAddr) );

		//first packet is forward. Set smallest packet so far to this one
		minPktSize_total.write(meta.flowIndex, (bit<32>)standard_metadata.packet_length);
		minPktSize_forward.write(meta.flowIndex, (bit<32>)standard_metadata.packet_length);

		//record timestamp for first packet
		firstTime_forward.write(meta.flowIndex, meta.intrinsic_metadata.ingress_global_timestamp);

		meta.flowFeatures.handshakeState = HandshakeState.initializing;

		isn_forward.write(meta.flowIndex, hdr.tcp.secNum);
	}

	//Calculate flow index
	action calcFlowIndex()
	{
		//both directions should point to same flow.
		//ignore cryptographic solution for now
		//TODO: CRYPTOGRAPHIC HASH!

		//calculate full flow index
		hash( meta.flowIndex, HashAlgorithm.crc32, (bit<32>)0, {hdr.ipv4.srcAddr+hdr.ipv4.dstAddr, hdr.tcp.srcPort+hdr.tcp.dstPort, hdr.udp.srcPort+hdr.udp.dstPort}, (bit<32>)FLOW_METADATA_MAXNUM);
	}


	table routetable
	{
		key = { 
			standard_metadata.ingress_port: exact;
			hdr.ipv4.dstAddr: exact;
			hdr.ipv4.srcAddr: exact;
			meta.flowFeatures.lastANNOutput: exact;
		}

		actions = {
			packetForward;
			packetDrop;
		}
	}
	

	/*
		A LOT of logic is placed directly in apply instead of actions, because actions don't support conditional statements
		It's super ugly, I know
	*/
	apply
	{
		//Get flow index for packet
		calcFlowIndex();

		/*
			Get last ANN output in flow, include check to see if ANN has performed inference, if not set to 0
			NN output goes from [1, x+1] instead of [0, x] to keep 0 reserved for "no inference performed"
		*/
		lastANNOutput.read(meta.flowFeatures.lastANNOutput, meta.flowIndex);
		

		routetable.apply();

		
		#if FLOW_RECORDING == 1 //if flow recording is enabled

			//tbl_pktCount_annTrigger.apply();
			pktCount_annTrigger.read(meta.pktCount_annTrigger, 0);
		
		

			//first check if a lock is necessary. Improves performance post-recording
			meta.collectFeatures = 0; //default to disable feature collection unless it gets enabled
			pktCount_total.read(meta.flowFeatures.pktCount_total, meta.flowIndex);
			if(meta.flowFeatures.pktCount_total < meta.pktCount_annTrigger)
			{
				//Ensure that only X packets in each flow are recorded
				meta.semaphore_id = 0; //lock semaphore 0 for this flow
				semaphore_lock(); //lock semaphore
					pktCount_total.read(meta.flowFeatures.pktCount_total, meta.flowIndex);

					//if this is one of the first X packets in flow
					if(meta.flowFeatures.pktCount_total < meta.pktCount_annTrigger)
					{
						//Update total number of flow packets            
						meta.flowFeatures.pktCount_total = meta.flowFeatures.pktCount_total +1;
						pktCount_total.write(meta.flowIndex, meta.flowFeatures.pktCount_total );

						//enabled feature collection
						meta.collectFeatures = 1;
					}
				semaphore_release(); //release semaphore
			}



			if(meta.collectFeatures == 1) //if feature collection is enabled
			{
			
				meta.semaphore_id = 1;
				semaphore_lock(); //lock semaphore
					//If first packet in flow
					if( meta.flowFeatures.pktCount_total == 1) 
					{
						initFlow(); //initialize flow
					
						//Set application protocol (in apply because actions don't support conditionals)
						//TODO: check application headers instead
						if(hdr.tcp.dstPort == 80 || hdr.tcp.dstPort == 443)
							aProto_http.write(meta.flowIndex, 1);
						else if(hdr.tcp.dstPort == 22)
							aProto_ssh.write(meta.flowIndex, 1);
						else if(hdr.tcp.dstPort == 21)
							aProto_ftp.write(meta.flowIndex, 1);
						else if(hdr.tcp.dstPort == 25)
							aProto_smtp.write(meta.flowIndex, 1);

							
					}
				semaphore_release();

				/*
					Flow-wide feature collection
				*/
				{
					//Update total data volume
					meta.semaphore_id = 2;
					semaphore_lock(); //lock semaphore
						dataVolume_total.read(meta.flowFeatures.dataVolume_total, meta.flowIndex);
						dataVolume_total.write(meta.flowIndex, meta.flowFeatures.dataVolume_total + (bit<64>)standard_metadata.packet_length );
					semaphore_release();
				
					//Update max packet size
					meta.semaphore_id = 3;
					semaphore_lock(); //lock semaphore
						maxPktSize_total.read(meta.flowFeatures.maxPktSize_total, meta.flowIndex);
						if( (bit<32>)standard_metadata.packet_length > meta.flowFeatures.maxPktSize_total ) //if new largest packet
						{
							maxPktSize_total.write(meta.flowIndex, (bit<32>)standard_metadata.packet_length);
						}
					semaphore_release();

					//Update min packet size
					meta.semaphore_id = 4;
					semaphore_lock(); //lock semaphore
						minPktSize_total.read(meta.flowFeatures.minPktSize_total, meta.flowIndex);
						if( (bit<32>)standard_metadata.packet_length < meta.flowFeatures.minPktSize_total ) //if new smallest packet
						{
							minPktSize_total.write(meta.flowIndex, (bit<32>)standard_metadata.packet_length);
						}
					semaphore_release();

					//Update timestamp of last packet in flow
					//meta.semaphore_id = 5;
					//semaphore_lock(); //lock semaphore
						//record timestamp for last packet
						lastTime_total.write(meta.flowIndex, meta.intrinsic_metadata.ingress_global_timestamp);
					//semaphore_release(); //release semaphore 5

					//Update total flow duration
					//meta.semaphore_id = 5;
					//semaphore_lock(); //lock semaphore
						firstTime_forward.read(meta.flowFeatures.firstTime_forward, meta.flowIndex);

						//calculate and write total flow duration
						flowDuration_total.write(meta.flowIndex, meta.intrinsic_metadata.ingress_global_timestamp - meta.flowFeatures.firstTime_forward);
					//semaphore_release(); //release semaphore 5

				}


				//Figure out if forward packet
				clientLowerIP.read(meta.flowFeatures.clientLowerIP, meta.flowIndex);
				meta.isForwardPacket = (bit<1>)( meta.flowFeatures.clientLowerIP == (bit<1>)(hdr.ipv4.srcAddr < hdr.ipv4.dstAddr) );

				/*
					Forward feature collection
				*/
				if( meta.isForwardPacket == 1 ) //if forward packet
				{
			
					//Update forward pkt counter
					meta.semaphore_id = 5;
					semaphore_lock(); //lock semaphore
						pktCount_forward.read(meta.flowFeatures.pktCount_forward, meta.flowIndex);
						pktCount_forward.write(meta.flowIndex, meta.flowFeatures.pktCount_forward +1 );
					semaphore_release();

					//Update data volume forward
					meta.semaphore_id = 6;
					semaphore_lock(); //lock semaphore
						dataVolume_forward.read(meta.flowFeatures.dataVolume_forward, meta.flowIndex);
						dataVolume_forward.write(meta.flowIndex, meta.flowFeatures.dataVolume_forward + (bit<64>)standard_metadata.packet_length );
					semaphore_release();

					//Update max packet size forward
					meta.semaphore_id = 7;
					semaphore_lock(); //lock semaphore
						maxPktSize_forward.read(meta.flowFeatures.maxPktSize_forward, meta.flowIndex);
						if( (bit<32>)standard_metadata.packet_length > meta.flowFeatures.maxPktSize_forward ) //if new largest packet
						{
							maxPktSize_forward.write(meta.flowIndex, (bit<32>)standard_metadata.packet_length);
						}
					semaphore_release();

					//Update min packet size forward
					meta.semaphore_id = 8;
					semaphore_lock(); //lock semaphore
						minPktSize_forward.read(meta.flowFeatures.minPktSize_forward, meta.flowIndex);
						if( (bit<32>)standard_metadata.packet_length < meta.flowFeatures.minPktSize_forward ) //if new smallest packet
						{
							minPktSize_forward.write(meta.flowIndex, (bit<32>)standard_metadata.packet_length);
						}
					semaphore_release();

					//Update timestamp of last packet forward
						//record timestamp for last packet
						lastTime_forward.write(meta.flowIndex, meta.intrinsic_metadata.ingress_global_timestamp);

					//Update last TCP seqnum forwards
						lastSeqnum_forward.write(meta.flowIndex, hdr.tcp.secNum);


					//Update RTT timestamps if TCP handshake not complete
					tproto_tcp.read(meta.flowFeatures.tproto_tcp, meta.flowIndex);
					if( meta.flowFeatures.tproto_tcp == 1 )//if this is TCP
					{
						meta.semaphore_id = 9;
						semaphore_lock(); //lock semaphore
							handshakeState.read(meta.flowFeatures.handshakeState, meta.flowIndex);
							if( meta.flowFeatures.handshakeState != HandshakeState.handshakeComplete ) //if handshake not complete
							{
								if( hdr.tcp.syn == 1 && hdr.tcp.ack == 0 ) //if this is SYN
								{
									handshakeSynTime.write(meta.flowIndex, meta.intrinsic_metadata.ingress_global_timestamp);
									handshakeState.write(meta.flowIndex, HandshakeState.handshakeSYN); //update handshakeState to SYN sent
								}
								else if( hdr.tcp.syn == 0 && hdr.tcp.ack == 1 && (meta.flowFeatures.handshakeState == HandshakeState.handshakeSYNACK) ) //if this is ACK
								{
									handshakeAckTime.write(meta.flowIndex, meta.intrinsic_metadata.ingress_global_timestamp);
									handshakeState.write(meta.flowIndex, HandshakeState.handshakeComplete); //update handshakeState to complete
								}
							}
						semaphore_release();
					}

					//Update last advertised congestion window of forward packet
					tcpWindow_forward.write(meta.flowIndex, hdr.tcp.cwndSize);

					//Update last ttl in ipv4 header forward
					ttl_forward.write(meta.flowIndex, hdr.ipv4.ttl);



					if(hdr.tcp.ack == 1) //If this is ACK packet
					{
						meta.semaphore_id = 16;
						semaphore_lock(); //lock semaphore
							//Update total number of forward ack packets
							acknum_forward.read(meta.flowFeatures.acknum_forward, meta.flowIndex);
							acknum_forward.write(meta.flowIndex, meta.flowFeatures.acknum_forward +1);
						semaphore_release(); //release semaphore
					}

				
				
				}
				/*
					Backward feature collection
				*/
				else //if backward packet
				{
			
					//Update backward pkt counter
					meta.semaphore_id = 10;
					semaphore_lock(); //lock semaphore
						pktCount_backward.read(meta.flowFeatures.pktCount_backward, meta.flowIndex);
						pktCount_backward.write(meta.flowIndex, meta.flowFeatures.pktCount_backward +1 );
					semaphore_release();

					//Update data volume backward
					meta.semaphore_id = 11;
					semaphore_lock(); //lock semaphore
						dataVolume_backward.read(meta.flowFeatures.dataVolume_backward, meta.flowIndex);
						dataVolume_backward.write(meta.flowIndex, meta.flowFeatures.dataVolume_backward + (bit<64>)standard_metadata.packet_length );
					semaphore_release();

					//Update max packet size backward
					meta.semaphore_id = 12;
					semaphore_lock(); //lock semaphore
						maxPktSize_backward.read(meta.flowFeatures.maxPktSize_backward, meta.flowIndex);
						if( (bit<32>)standard_metadata.packet_length > meta.flowFeatures.maxPktSize_backward ) //if new largest packet
						{
							maxPktSize_backward.write(meta.flowIndex, (bit<32>)standard_metadata.packet_length);
						}
					semaphore_release();

					//Update min packet size backward
					meta.semaphore_id = 13;
					semaphore_lock(); //lock semaphore
						minPktSize_backward.read(meta.flowFeatures.minPktSize_backward, meta.flowIndex);
						if( (bit<32>)standard_metadata.packet_length < meta.flowFeatures.minPktSize_backward || meta.flowFeatures.pktCount_backward == 0 ) //if new smallest packet (or first backward packet)
						{
							minPktSize_backward.write(meta.flowIndex, (bit<32>)standard_metadata.packet_length);
						}
					semaphore_release();

					//Update timestamp of last packet backward
						if( meta.flowFeatures.pktCount_backward == 0 ) //if first packet
						{
							//update timestamp for first backwards packet
							firstTime_backward.write(meta.flowIndex, meta.intrinsic_metadata.ingress_global_timestamp);
						}

						//record timestamp for last packet
						lastTime_backward.write(meta.flowIndex, meta.intrinsic_metadata.ingress_global_timestamp);

					//Update last TCP seqnum backward
						lastSeqnum_backward.write(meta.flowIndex, hdr.tcp.secNum);

						if(meta.flowFeatures.pktCount_backward == 0) //if first packet
						{
							//Also set ISN to seqnum of first backward packet
							isn_backward.write(meta.flowIndex, hdr.tcp.secNum);
						}

					//Update RTT timestamps if TCP handshake not complete (backwards only send SYNACK packet)
					tproto_tcp.read(meta.flowFeatures.tproto_tcp, meta.flowIndex);
					if( meta.flowFeatures.tproto_tcp == 1 )//if this is TCP
					{
						meta.semaphore_id = 9; //same semaphore id for RTT in both directions
						semaphore_lock(); //lock semaphore
							handshakeState.read(meta.flowFeatures.handshakeState, meta.flowIndex);
							if( meta.flowFeatures.handshakeState != HandshakeState.handshakeComplete ) //if handshake not complete
							{
								if( hdr.tcp.syn == 1 && hdr.tcp.ack == 1  && (meta.flowFeatures.handshakeState == HandshakeState.handshakeSYN) ) //if this is SYNACK
								{
									handshakeSynackTime.write(meta.flowIndex, meta.intrinsic_metadata.ingress_global_timestamp);
									handshakeState.write(meta.flowIndex, HandshakeState.handshakeSYNACK); //update handshakeState to SYN sent
								}
							}
						semaphore_release();
					}

					//Update last advertised congestion window of backward packet
						tcpWindow_backward.write(meta.flowIndex, hdr.tcp.cwndSize);

					//Update last ttl in ipv4 header backward
						ttl_backward.write(meta.flowIndex, hdr.ipv4.ttl);

	
					if(hdr.tcp.ack == 1) //If this is ACK packet
					{
						meta.semaphore_id = 16;
						semaphore_lock(); //lock semaphore
							//Update total number of forward ack packets
							acknum_backward.read(meta.flowFeatures.acknum_backward, meta.flowIndex);
							acknum_backward.write(meta.flowIndex, meta.flowFeatures.acknum_backward +1);
						semaphore_release(); //release semaphore
					}

				}


				//Update total number of packets recorded
				meta.semaphore_id = 14; //lock semaphore 14 for this flow
				semaphore_lock(); //lock semaphore
					pktCount_collectedFeatures.read(meta.flowFeatures.pktCount_collectedFeatures, meta.flowIndex);
					meta.flowFeatures.pktCount_collectedFeatures = meta.flowFeatures.pktCount_collectedFeatures +1;
					pktCount_collectedFeatures.write(meta.flowIndex, meta.flowFeatures.pktCount_collectedFeatures );
				semaphore_release(); //release semaphore
			
			}

		#endif //end for if FLOW_RECORDING == 1


	}

	
}


control MyEgress(inout headers hdr, inout metadata meta, inout standard_metadata_t standard_metadata)
{
	//Read features from registers into metadata
	action featuresToMetadata()
	{
		dataVolume_forward.read(meta.flowFeatures.dataVolume_forward, meta.flowIndex);
		serverAddr.read(meta.flowFeatures.serverAddr, meta.flowIndex);
		pktCount_total.read(meta.flowFeatures.pktCount_total, meta.flowIndex);
	}

	action performNeuralNetworkInference()
	{
		featuresToMetadata(); //read feature registers to metadata

		neuralNetworkEval();

		//store output from ANN in register for flow
		lastANNOutput.write(meta.flowIndex, meta.flowFeatures.lastANNOutput);
	}

	action triggerNeuralNetworkInference()
	{
		//If cloning technique disabled, just perform in regular packet pipeline
		performNeuralNetworkInference();

	}

	action setANNTriggerInterval(bit<32> annTriggerInterval)
	{
		meta.annTriggerInterval = annTriggerInterval;
	}

	apply
	{
		performNeuralNetworkInference(); //Used while measuring  latency (infer on every packet)

		#if ANN_EVAL == 1

			bit<1> performNNInference;


			//if this is the packet which should trigger an inference (set inside mutexlock, so no need here)
			if( meta.flowFeatures.pktCount_collectedFeatures == meta.pktCount_annTrigger )
			{
				triggerNeuralNetworkInference();
			}
		
		
			
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


















