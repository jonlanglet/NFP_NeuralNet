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
#ifndef __LANGLET_FLOATS_H_
	#define __LANGLET_FLOATS_H_


	#include <stdint.h>


	/*
		Convert a float to integer representation:
		example 0.2: 0.2*2^15 = 6553 and -0.2: -6553

		storing floats in 64-bit all the way through seems to be the most speed-optimized solution, but the most memory heavy
		Latency test would be nice
	*/

	//make sure this is correct! 2<<30 = 2^31
	#define FLOAT_SHIFT 15//30 //15 //2^15 = 32768 | this results in accuracy 2^-15 ~= 3*10^-5 ~== 0.00003  (acceptable-ish)

	typedef int64_t float_t; //base floats on 64-bit integers (this feels so wrong)



	/*
		A list of x-bit shifted float values
		Use these for debugging and specifying lookup tables
		ROUND UP WHEN REQUIRED!
	*/
	#if FLOAT_SHIFT == 15
		#define val0         0
		#define val0_001     33
		#define val0_005     164
		#define val0_01      328
		#define val0_5       16384
		#define val1_0       32768
		#define val2         65536
		#define val6         196608
		#define val10        327680
	#endif

	#if FLOAT_SHIFT == 20
		#define val0        0
		#define val0_5      524288
		#define val1_0      1048576
		#define val2      2097152
		#define val6      6291456
		#define val10       10485760
	#endif
	#if FLOAT_SHIFT == 30
		#define val0        0           //0
		#define val0_01     10737418    //0.01
		#define val0_1      107374182   //0.1
		#define val0_25     268435456   //0.25
		#define val0_5      536870912   //0.5
		#define val0_75     805306368   //0.75
		#define val1_0      1073741824  //1.0
	#endif

	
	/*
		Function prototypes
	*/
	float_t fMult(float_t a, float_t b);



	/*
	Sigmoid implementation using lookup tables
	*/
	//Threshhold between lookup table intervals (splitting points)
	#define SIGTHRESH1 -val10
	#define SIGTHRESH2 -val6
	#define SIGTHRESH3 -val2
	#define SIGTHRESH4 val2
	#define SIGTHRESH5 val6
	#define SIGTHRESH6 val10

	#define SIGSTEPSIZE1 val0_01
	#define SIGSTEPSIZE2 val0_005
	#define SIGSTEPSIZE3 val0_001
	#define SIGSTEPSIZE4 val0_005
	#define SIGSTEPSIZE5 val0_01

	//Size of lookup tables
	#define SIGTABSIZE1 400 //-10 to -6, 0.01 steps => 400 entries
	#define SIGTABSIZE2 800 //-6 to -2, 0.005 steps => 800 entries
	#define SIGTABSIZE3 4000 //-2 to 2, 0.001 steps => 4000 entries
	#define SIGTABSIZE4 800 //2 to 6, 0.005 steps => 800 entries
	#define SIGTABSIZE5 400 //6 to 10, 0.01 steps => 400 entries

	//The actual lookup tables
	volatile _declspec(imem export scope(global)) float_t sigmoidTab1[SIGTABSIZE1];
	volatile _declspec(imem export scope(global)) float_t sigmoidTab2[SIGTABSIZE2];
	volatile _declspec(imem export scope(global)) float_t sigmoidTab3[SIGTABSIZE3];
	volatile _declspec(imem export scope(global)) float_t sigmoidTab4[SIGTABSIZE4];
	volatile _declspec(imem export scope(global)) float_t sigmoidTab5[SIGTABSIZE5];


	/*
		Returns an approximation of sigmoid(x) = 1 / (1 + e^-x)
		Using a step-function and lookup-tables
	*/
	float_t sigmoid(float_t x);


#endif