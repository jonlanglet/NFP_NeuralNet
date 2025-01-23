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

import random

layer1Size = 5
layer2Size = 25
layer3Size = 20
layer4Size = 15
layer5Size = 5

val1_0 = 2**30
 
f = open("neuralNetworkParams.c", "w+");

f.write("#include \"neuralNetwork.h\"\n\n");
f.write("void func1()\n{\n");

for i in range(layer1Size):
	for j in range(layer2Size):
		f.write("\tweights_1_2[%d][%d] = %d;\n" %(i, j, random.randint(-val1_0, val1_0) ) );
f.write("\n}\n");

f.write("void func2()\n{\n");
for i in range(layer2Size):
	for j in range(layer3Size):
		f.write("\tweights_2_3[%d][%d] = %d;\n" %(i, j, random.randint(-val1_0, val1_0) ) );
f.write("\n}\n");

f.write("void func3()\n{\n");
for i in range(layer3Size):
	for j in range(layer4Size):
		f.write("\tweights_3_4[%d][%d] = %d;\n" %(i, j, random.randint(-val1_0, val1_0) ) );
f.write("\n}\n");

f.write("void func4()\n{\n");
for i in range(layer4Size):
	for j in range(layer5Size):
		f.write("\tweights_4_5[%d][%d] = %d;\n" %(i, j, random.randint(-val1_0, val1_0) ) );
f.write("\n}\n\n");

f.write("void func5()\n{\n");
for i in range(layer2Size):
	f.write("\tbias_1_2[%d] = %d;\n" %(i, random.randint(-val1_0, val1_0) ));
f.write("\n");

for i in range(layer3Size):
	f.write("\tbias_2_3[%d] = %d;\n" %(i, random.randint(-val1_0, val1_0) ));
f.write("\n");

for i in range(layer4Size):
	f.write("\tbias_3_4[%d] = %d;\n" %(i, random.randint(-val1_0, val1_0) ));
f.write("\n");

for i in range(layer5Size):
	f.write("\tbias_4_5[%d] = %d;\n" %(i, random.randint(-val1_0, val1_0) ));
f.write("\n}\n\n");

f.write("void setAnnParameters()\n{\n");
f.write("\tfunc1();\n");
f.write("\tfunc2();\n");
f.write("\tfunc3();\n");
f.write("\tfunc4();\n");
f.write("\tfunc5();\n");
f.write("\n}\n");
