#include "neuralNetwork.h"

/*
    Returns a random parameter value
*/
float_t randomParam()
{
    float_t val;
    val = rand32() % val1_0; //local_csr_read(local_csr_pseudo_random_number);//rand32();

    if(rand32()%2==1)
        val = -val;

    return val;
}

void setAnnParameters()
{
    int i, j;

    //Initialize random function
    srand32(0);

    /*
    Set random weights
    */
    for(i = 0; i < INPUT_NNODES; i++) //from i
        for(j = 0; j < HIDDEN1_NNODES; j++) //to j
            weights_I_H1[i][j] = randomParam();

    #if ANN_HLAYER_NUM >= 2
        for(i = 0; i < HIDDEN1_NNODES; i++)
            for(j = 0; j < HIDDEN2_NNODES; j++)
                weights_H1_H2[i][j] = randomParam();
    #endif

    #if ANN_HLAYER_NUM >= 3
        for(i = 0; i < HIDDEN2_NNODES; i++)
            for(j = 0; j < HIDDEN3_NNODES; j++)
                weights_H2_H3[i][j] = randomParam();
    #endif

    #if ANN_HLAYER_NUM >= 4
        for(i = 0; i < HIDDEN3_NNODES; i++)
            for(j = 0; j < HIDDEN4_NNODES; j++)
                weights_H3_H4[i][j] = randomParam();
    #endif

    //randomize weights to output layer
    #if ANN_HLAYER_NUM == 1
        for(i = 0; i < HIDDEN1_NNODES; i++)
            for(j = 0; j < OUTPUT_NNODES; j++)
                weights_H_O[i][j] = randomParam();
    #endif
    #if ANN_HLAYER_NUM == 2
        for(i = 0; i < HIDDEN2_NNODES; i++)
            for(j = 0; j < OUTPUT_NNODES; j++)
                weights_H_O[i][j] = randomParam();
    #endif
    #if ANN_HLAYER_NUM == 3
        for(i = 0; i < HIDDEN3_NNODES; i++)
            for(j = 0; j < OUTPUT_NNODES; j++)
                weights_H_O[i][j] = randomParam();
    #endif
    #if ANN_HLAYER_NUM == 4
        for(i = 0; i < HIDDEN4_NNODES; i++)
            for(j = 0; j < OUTPUT_NNODES; j++)
                weights_H_O[i][j] = randomParam();
    #endif

    /*
    Set random biases
    */
    for(i = 0; i < HIDDEN1_NNODES; i++)
        bias_I_H1[i] = randomParam();

    #if ANN_HLAYER_NUM >= 2
        for(i = 0; i < HIDDEN2_NNODES; i++)
            bias_H1_H2[i] = randomParam();
    #endif
    
    #if ANN_HLAYER_NUM >= 3
        for(i = 0; i < HIDDEN3_NNODES; i++)
            bias_H2_H3[i] = randomParam();
    #endif

    #if ANN_HLAYER_NUM >= 4
        for(i = 0; i < HIDDEN4_NNODES; i++)
            bias_H3_H4[i] = randomParam();
    #endif

    for(i = 0; i < OUTPUT_NNODES; i++)
        bias_H_O[i] = randomParam();

}
