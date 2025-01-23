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
#include <mem_atomic.h>

#include "floats.h"
#include "flowHandling.h"

#include "neuralNetwork.h"

#include <stdlib.h>









/*
    Activation function goes here
    
    sigmoidal example:   f(x) = 1 / ( 1 + e^(-x) )

    But should return floats as integer representations!

    
    Should I try ReLU instead? Much easier to implement in this architecture
    example ReLU: f(x) = max(0, x)
    Only use ReLU in hidden layers. (softmax to output? Simply normalize output layer))

    Leaky ReLU preferable to avoid dying neurons during training?
    example: 
    x < 0 => f(x) = 0.01x
    x > 0 => f(x) = x;
*/
float_t activationFunc(float_t x)
{
    
    //ReLU
    if(x > 0) return x;
    else return 0;
    

    
    /*
    //Leaky ReLU
    if(x >= 0) return x;
    else return fMult(x, val0_01); //return x * 0.01, represented as integers
    */
    
    /*
    //ReLU-6
    if(x < 0) return 0;
    else if(x > 6) return 6;
    else return x;
    */
    
    
    
    //return x;
}

/*
    update the activation-arrays from activations in first layer
    result output will be in last activation layer
*/
uint8_t evaluateInput()
{
    uint8_t i, j;
    __lmem uint8_t highestOutNeuron = 0;
    __lmem float_t highestOutNeuron_activation;

    //calculate activations in hidden layer 1
    for(i = 0; i < HIDDEN1_NNODES; i++)
    {
        activations_H1[i] = 0; //reset hidden layer neurons before calculation

        for(j = 0; j < INPUT_NNODES; j++)
            activations_H1[i] += fMult(activations_I[j], weights_I_H1[j][i]); //add each node's activation*weight

        //add bias
        activations_H1[i] += bias_I_H1[i];

        //run through activation function
        activations_H1[i] = activationFunc(activations_H1[i]);
    }

    
    //calculate activations in hidden layer 2
    for(i = 0; i < HIDDEN2_NNODES; i++)
    {
        activations_H2[i] = 0;

        for(j = 0; j < HIDDEN1_NNODES; j++)
            activations_H2[i] += fMult(activations_H1[j], weights_H1_H2[j][i]); //add each node's activation*weight

        //add bias
        activations_H2[i] += bias_H1_H2[i];

        //run through activation function
        activations_H2[i] = activationFunc(activations_H2[i]);
    }

    //calculate activations in hidden layer 3
    #if ANN_HLAYER_NUM >= 3
        for(i = 0; i < HIDDEN3_NNODES; i++)
        {
            activations_H3[i] = 0;

            for(j = 0; j < HIDDEN2_NNODES; j++)
                activations_H3[i] += fMult(activations_H2[j], weights_H2_H3[j][i]); //add each node's activation*weight

            //add bias
            activations_H3[i] += bias_H2_H3[i];

            //run through activation function
            activations_H3[i] = activationFunc(activations_H3[i]);
        }
    #endif

    //calculate activations in hidden layer 4
    #if ANN_HLAYER_NUM >= 4
        for(i = 0; i < HIDDEN3_NNODES; i++)
        {
            activations_H4[i] = 0;

            for(j = 0; j < HIDDEN3_NNODES; j++)
                activations_H4[i] += fMult(activations_H3[j], weights_H3_H4[j][i]); //add each node's activation*weight

            //add bias
            activations_H4[i] += bias_H3_H4[i];

            //run through activation function
            activations_H4[i] = activationFunc(activations_H4[i]);
        }
    #endif

    //calculate activations for output layer, and find highest value
    for(i = 0; i < OUTPUT_NNODES; i++)
    {
        activations_O[i] = 0;

        #if ANN_HLAYER_NUM == 3
            for(j = 0; j < HIDDEN3_NNODES; j++)
                activations_O[i] += fMult(activations_H3[j], weights_H_O[j][i]); //add each node's activation*weight
        #endif
        #if ANN_HLAYER_NUM == 4
            for(j = 0; j < HIDDEN4_NNODES; j++)
                activations_O[i] += fMult(activations_H4[j], weights_H_O[j][i]); //add each node's activation*weight
        #endif

        //add bias
        activations_O[i] += bias_H_O[i];

        //run through activation function
        activations_O[i] = activationFunc(activations_O[i]);

        //if this is the highest activation so far
        if( activations_O[i] > highestOutNeuron_activation )
        {
            highestOutNeuron_activation = activations_O[i];
            highestOutNeuron = i;
        }

    }


    return highestOutNeuron;

}




/*
    gathered from pif_plugin_metadata.h

    Parameters: (header)
    Most values have a setter as well: (header, (int)newVal)

    pif_plugin_meta_get__standard_metadata__packet_length(headers);
    pif_plugin_meta_get__standard_metadata__ingress_port(headers);

*/

int pif_plugin_neuralNetworkEval(EXTRACTED_HEADERS_T *headers, ACTION_DATA_T *action_data)
{
    index_t flowIndex;
    uint8_t annResult;

    //These are optimized away, so keep them for legibility
    PIF_PLUGIN_ethernet_T *ethHeader = pif_plugin_hdr_readonly_get_ethernet(headers);
    PIF_PLUGIN_ipv4_T *ipv4Header = pif_plugin_hdr_readonly_get_ipv4(headers);
    PIF_PLUGIN_ipv6_T *ipv6Header = pif_plugin_hdr_readonly_get_ipv6(headers);
    PIF_PLUGIN_tcp_T *tcpHeader = pif_plugin_hdr_readonly_get_tcp(headers);
    PIF_PLUGIN_udp_T *udpHeader = pif_plugin_hdr_readonly_get_udp(headers);    


    flowIndex = calculateFlowIndex(headers);

    
    //Only evaluate flow every 20:th pkt in this traffic flow
    //Race conditions can make this execute multiple times (unlikely, but possible)
    //Semaphore around these 3 lines should solve the problem with race conditions
    if( flowMetadata[flowIndex].pktCount - flowMetadata[flowIndex].lastANNCheck < ANN_ANALYZE_INTERVAL )
        return PIF_PLUGIN_RETURN_FORWARD;
    flowMetadata[flowIndex].lastANNCheck = flowMetadata[flowIndex].pktCount;

    
    //calculate and update flow metadata from recorded values
    prepareForANN(flowIndex); 


    //set input values to ANN
    activations_I[0] = flowIndex;
    activations_I[1] = flowMetadata[flowIndex].pktCount_forward;
    activations_I[2] = flowMetadata[flowIndex].pktCount_backward;
    activations_I[3] = flowMetadata[flowIndex].meanPktSize_forward;
    activations_I[4] = flowMetadata[flowIndex].meanPktSize_backward;
    //activations_I[5] = getTotalFlowData_forward(flowIndex);
    //activations_I[6] = getTotalFlowData_backward(flowIndex);
    //activations_I[7] = flowMetadata[flowIndex].tcpWindow_source;
    //activations_I[8] = flowMetadata[flowIndex].tcpWindow_dest;
    //activations_I[9] = flowMetadata[flowIndex].flowDuration;


    annResult = evaluateInput();

    flowMetadata[flowIndex].trafficClass = annResult; //update the flow metadata-table with the calculated traffic-class


    return PIF_PLUGIN_RETURN_FORWARD;
}


/*
    Init-functions
*/
void pif_plugin_init_master() //called once system-wide
{
    setAnnParameters();
}


void pif_plugin_init() //called once for each worker thread
{
    
}













