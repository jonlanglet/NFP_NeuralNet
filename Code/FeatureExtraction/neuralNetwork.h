#ifndef __NEURAL_NETWORK_H_
    #define __NEURAL_NETWORK_H_


    #include "floats.h"
    #include "misc.h"
    #include "config.h"


    
    typedef _declspec(ANN_WEIGHT_REGION export scope(island)) float_t ann_weight_t; //every thread can share the same weights. Export to keep latency down
    typedef _declspec(ANN_ACTIVATION_REGION) float_t ann_activation_t; //Every thread needs its own span i for activations



    /*
        Allocating memory for ANN
    */
    //weights from input layer
    ann_weight_t weights_I_H1[INPUT_NNODES][HIDDEN1_NNODES];

    //weights between hidden layers
    #if ANN_HLAYER_NUM >= 2
        ann_weight_t weights_H1_H2[HIDDEN1_NNODES][HIDDEN2_NNODES];
    #endif
    #if ANN_HLAYER_NUM >= 3
        ann_weight_t weights_H2_H3[HIDDEN2_NNODES][HIDDEN3_NNODES];
    #endif
    #if ANN_HLAYER_NUM >= 4
        ann_weight_t weights_H3_H4[HIDDEN2_NNODES][HIDDEN3_NNODES];
    #endif

    //weights to output layer
    #if ANN_HLAYER_NUM == 1
        ann_weight_t weights_H_O[HIDDEN1_NNODES][OUTPUT_NNODES];
    #endif
    #if ANN_HLAYER_NUM == 2
        ann_weight_t weights_H_O[HIDDEN2_NNODES][OUTPUT_NNODES];
    #endif
    #if ANN_HLAYER_NUM == 3
        ann_weight_t weights_H_O[HIDDEN3_NNODES][OUTPUT_NNODES];
    #endif
    #if ANN_HLAYER_NUM == 4
        ann_weight_t weights_H_O[HIDDEN4_NNODES][OUTPUT_NNODES];
    #endif

    //bias from input layer
    ann_weight_t bias_I_H1[HIDDEN1_NNODES];
    //bias between hidden layers
    #if ANN_HLAYER_NUM >= 2
        ann_weight_t bias_H1_H2[HIDDEN2_NNODES];
    #endif
    #if ANN_HLAYER_NUM >= 3
        ann_weight_t bias_H2_H3[HIDDEN3_NNODES];
    #endif
    #if ANN_HLAYER_NUM >= 4
        ann_weight_t bias_H3_H4[HIDDEN4_NNODES];
    #endif
    //bias to output layer
    ann_weight_t bias_H_O[OUTPUT_NNODES];

    //activation in input layer
    ann_activation_t activations_I[INPUT_NNODES];
    //activations in hidden layers
    ann_activation_t activations_H1[HIDDEN1_NNODES];
    #if ANN_HLAYER_NUM >= 2
        ann_activation_t activations_H2[HIDDEN2_NNODES];
    #endif
    #if ANN_HLAYER_NUM >= 3
        ann_activation_t activations_H3[HIDDEN3_NNODES];
    #endif
    #if ANN_HLAYER_NUM >= 4
        ann_activation_t activations_H4[HIDDEN4_NNODES];
    #endif
    //activations in output layer
    ann_activation_t activations_O[OUTPUT_NNODES];



    //Function prototypes
    void setAnnParameters();

#endif
















