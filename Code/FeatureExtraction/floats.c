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

#include "floats.h"

/*
	a and b should be integer representations of signed float
	example: 0.2 = 0.2<<15 = 6553 and -0.2 = -6553

	returns integer representation of the float product


	TODO: use high and low registers for multiplication, and the increase FLOAT_SHIFT for smaller units
*/
float_t fMult(float_t a, float_t b)
{
	//32-bit integers (solution 1)
	//int64_t product = (int64_t)a * (int64_t)b;
	//return product >> FLOAT_SHIFT;
	
	//32-bit integers (solution 2)
	//return ( (int64_t)a * (int64_t)b ) >> FLOAT_SHIFT;

	//64-bit integers (solution 3) (fewest assembly instructions)
	int64_t product = a * b;
	return product >> FLOAT_SHIFT;

	//64-bit integers (solution 4)
	//return (a*b) >> FLOAT_SHIFT;
}


float_t lookup1(float_t x)
{
	uint16_t index;
	index = (x - SIGTHRESH1) / SIGSTEPSIZE1; //division should round down

	return sigmoidTab1[index];
}
float_t lookup2(float_t x)
{
	uint16_t index;
	index = (x - SIGTHRESH2) / SIGSTEPSIZE2; //division should round down

	return sigmoidTab2[index];
}
float_t lookup3(float_t x)
{
	uint16_t index;
	index = (x - SIGTHRESH3) / SIGSTEPSIZE3; //division should round down

	return sigmoidTab3[index];
}
float_t lookup4(float_t x)
{
	uint16_t index;
	index = (x - SIGTHRESH4) / SIGSTEPSIZE4; //division should round down

	return sigmoidTab4[index];
}
float_t lookup5(float_t x)
{
	uint16_t index;
	index = (x - SIGTHRESH5) / SIGSTEPSIZE5; //division should round down

	return sigmoidTab5[index];
}

/*
	Sigmoid approximating step-function using lookup tables
*/
float_t sigmoid(float_t x)
{
	//Broken down into intervals with different lookup tables (different step sizes)
	if( x < SIGTHRESH1 )
		return 0;
	else if( x < SIGTHRESH2 )
		return lookup1(x);
	else if( x < SIGTHRESH3 )
		return lookup2(x);
	else if( x < SIGTHRESH4 )
		return lookup3(x);
	else if( x < SIGTHRESH5 )
		return lookup4(x);
	else if( x < SIGTHRESH6 )
		return lookup5(x);
	else
		return 1;
}