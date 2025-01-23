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
#ifndef __LANGLET_CONFIG_H
	#define __LANGLET_CONFIG_H


	/*
		Enabled components
	*/
	#define FLOW_RECORDING 1 //if the flow metadata should be recorded and stored
	#define ANN_EVAL 1 //if there should be any ANN evals at all. Useful when measuring impact

	#define CLONE_FOR_ANN_EVAL 1 //enable to emit packet directly, then analyzing a clone instead of original. Greatly reduces latency. Disable when measuring ANN footprint

	//#define ANN_IN_INGRESS 0 //enable to move ANN from egress->ingress processing. Not compatible with cloning! At most one of these should be enabled

	
	#define ANN_ANALYZE_INTERVAL 20 //Analyze a flow every X packets through it



	/*
		Set static ANN structure at compile-time
	*/
	#define ANN_HLAYER_NUM 3 //Number of hidden layers in ANN (supports 2,3,4)

	//Set number of neurons in each layer
	#define INPUT_NNODES 5 //Number of input neurons
	#define HIDDEN1_NNODES 10 //Number of neurons in hidden layer 1
	#define HIDDEN2_NNODES 10 //Number of neurons in hidden layer 2
	#define HIDDEN3_NNODES 10 //Number of neurons in hidden layer 3 (if it exists)
	#define HIDDEN4_NNODES 0 //Number of neurons in hidden layer 4 (if it exists)
	#define OUTPUT_NNODES 5 //Number of output neurons


	/*
		What memory regions to place ANN variables
		CLS activations in combination with clone results in hwdbg crash!
	*/
	#define ANN_WEIGHT_REGION ctm //local_mem, cls, ctm, imem, emem
	#define ANN_ACTIVATION_REGION ctm //local_mem, cls, ctm, imem, emem

#endif

