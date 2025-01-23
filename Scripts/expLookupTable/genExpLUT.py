'''
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
'''

import math


val1_0 = 1073741824.0 #this is 1 in my float implementation

maxVal = val1_0*10 #maximum exponent. will go here linearly

maxIndex = 4095 #number of entries in lookup tables

width = 5

numFunctions = 10 #number of functions to break each initialization into (or compile error because too large function)

stepSize = maxVal/maxIndex

f = open("floats_expLookupTbl.c", "w+");


f.write("/*GENERATED FILE*/\n");

f.write("//#define EXPLOOKUP_NUMENTRIES %d //Number of entries generated\n" %maxIndex);
f.write("//#define EXPLOOKUP_STEPSIZE %d\n\n" %( stepSize ))
f.write("#include \"floats.h\"\n\n");

#f.write("\tstatic float_t expLookupTable_pos[EXPLOOKUP_NUMENTRIES] = {");


i = 0
curVal = 0
curFunc = 0
f.write("void initExpLookupTable_pos0()\n{\n");
while curVal <= maxVal and i < maxIndex:
	if i % (maxIndex/numFunctions) == 0 and i != 0:
		curFunc = curFunc+1
		f.write("\n}\n\nvoid initExpLookupTable_pos%d()\n{\n" %curFunc);
	elif i % width == 0 and i != 0:
		f.write("\n");
	i = i + 1
	exp = curVal / val1_0 
	f.write( "\texpLookupTable_pos[%d] = %d;/*%f*/\t" %( i, math.exp(exp) *val1_0, exp ) )
	curVal = curVal + stepSize
f.write("\n}\n\n");


i = 0
curVal = 0
curFunc = 0
f.write("void initExpLookupTable_neg0()\n{\n");
while curVal >= -maxVal and i < maxIndex:
	if i % (maxIndex/numFunctions) == 0 and i != 0:
		curFunc = curFunc+1
		f.write("\n}\n\nvoid initExpLookupTable_neg%d()\n{\n" %curFunc);
	elif i % width == 0 and i != 0:
		f.write("\n");
	i = i + 1
	exp = curVal / val1_0 
	f.write( "\texpLookupTable_neg[%d] = %d;/*%f*/\t" %( i, math.exp(exp) *val1_0, exp ) )
	curVal = curVal - stepSize
f.write("\n}\n\n");



f.write("void initExpLookupTable()\n{\n");
for i in range(numFunctions):
	f.write("\tinitExpLookupTable_pos%d();\n" %i);
	f.write("\tinitExpLookupTable_neg%d();\n" %i);
f.write("\n}");













