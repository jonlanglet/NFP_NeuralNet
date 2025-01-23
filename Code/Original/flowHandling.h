#ifndef __FLOW_HANDLING_H_
    #define __FLOW_HANDLING_H_


    //#define SIZE_FLOWTABLE 1048576 //how big the flow hashtable should be. Max number of entries
    #define SIZE_FLOWTABLE 16 //128

    /*
        Set datatypes
    */
    typedef uint32_t index_t; //stores index to flowTable
    typedef uint32_t pktsize_t; //stores size of packet
    typedef uint32_t pktcount_t; //stores packet count
    typedef uint64_t time_t; //stores timestamps

    /*
        Will store meta-information about the packet flow
        volatile forces compiler not to optimize the variables away

        TODO: change to p4 hashtables?
    */
    struct FlowMetadata
    {
        volatile uint8_t trafficClass; //what was the last classification made by the neural network?
        volatile pktcount_t lastANNCheck; //what pktCount the ANN last analyzed the flow

        volatile uint32_t clientAddr;
        volatile uint32_t serverAddr;

        volatile uint16_t clientPort;
        volatile uint16_t serverPort;

        volatile uint8_t clientLowerIP; //set to 1 if clientIP < serverIP. Used to handle the directions separately

        volatile time_t initTime; //timestamp for first flow pkt
        volatile time_t lastTime; //timestamp for last flow pkt

        volatile pktcount_t pktCount;
        volatile pktcount_t pktCount_forward; //from "client" to "server"
        volatile pktcount_t pktCount_backward; //from "server" to "client"

        //Stores total data in bytes
        volatile uint64_t totalFlowData; //High & low part are swapped! Remember this when using the values
        volatile uint64_t totalFlowData_forward;
        volatile uint64_t totalFlowData_backward;
        
        volatile uint8_t tproto_tcp;
        volatile uint8_t tproto_udp;

        volatile pktsize_t maxPktSize;
        volatile pktsize_t maxPktSize_forward;
        volatile pktsize_t maxPktSize_backward;

        volatile pktsize_t minPktSize;
        volatile pktsize_t minPktSize_forward;
        volatile pktsize_t minPktSize_backward;

        volatile uint8_t aProto_http; //is the application protocl http?
        volatile uint8_t aProto_ssh;
        volatile uint8_t aProto_ftp;

        volatile uint32_t lastSeqnum_forward; //the last seqnum seen forwards in flow
        volatile uint32_t lastSeqnum_backward;

        volatile time_t synTime;
        volatile time_t synackTime;

        volatile time_t flowDuration;
        volatile uint64_t load_forward; //data/sec forwards (in bytes/second)
        volatile uint64_t load_backward; //data/sec backwards (bytes/second)
        volatile uint8_t ttl_forward; //ttl of last forward packet
        volatile uint8_t ttl_backward; //ttl of last backward packet
        volatile pktcount_t retrans_forward; //# of retransmissions forwards
        volatile pktcount_t retrans_backward;
        volatile time_t interPktGap_forward; //interpacket arrival time forwards
        volatile time_t interPktGap_backward;
        //volatile time_t jitter_forward;
        //volatile time_t jitter_backward;
        volatile uint16_t tcpWindow_source; //advertised tcp window
        volatile uint16_t tcpWindow_dest;
        volatile uint32_t ISN_forward; //initial sequence number for source
        volatile uint32_t ISN_backward;
        volatile time_t tcpSynSynackTime; //time between syn & synack
        volatile time_t tcpSynackAckTime; //time between synack & ack
        volatile time_t tcpSetupRTT; //syn + synack times (tcpSynSynackTime + tcpSynackAckTime)
        volatile uint32_t meanPktSize_forward; //mean size of forward packets in flow
        volatile uint32_t meanPktSize_backward;

    };

    volatile _declspec(imem export scope(global)) struct FlowMetadata flowMetadata[SIZE_FLOWTABLE]; 


    /*
        Function prototypes
    */
    index_t calculateFlowIndex(EXTRACTED_HEADERS_T *headers);
    int pif_plugin_registerPacket(EXTRACTED_HEADERS_T *headers, ACTION_DATA_T *action_data);
    void prepareForANN(index_t flowIndex);

    //getters
    uint64_t getTotalFlowData_forward(index_t flowIndex);
    uint64_t getTotalFlowData_backward(index_t flowIndex);


#endif