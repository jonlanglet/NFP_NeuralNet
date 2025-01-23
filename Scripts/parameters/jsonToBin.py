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
import json
import math


#Configuration
json_file = 'ann.json'
outputDir = "output/"

#Has to be consistent in Micro-C config files, generated SIgmoid tables, and here
bitShift = 15

modelName = "systemTest"


#Convert from floats into fixed-point representation of those (using bit-shift 15)
def fixedPoint(x):
	return int(x * math.pow(2, bitShift))



ann = {}


with open(json_file) as json_data:
	data = json.load(json_data)


#Parse JSON into lists
weights = data["ANN"][0]["Weights"]
for currentLayer in weights:
	print("\n\nProcessing layer %s..." % currentLayer)
	
	#Create structure for current layer
	ann[currentLayer] = {} 
	ann[currentLayer]["Weights"] = {}
	ann[currentLayer]["Biases"] = {}
	
	#Loop through destination nodes
	for nodes in weights[currentLayer]:
		for node in nodes:
			print("\tStructuring node %s parameters..." % node)
			
			for weight in nodes[node]:
				
				index = list(weight)[0]
				
				val = list(weight.values())[0]
				valFixed = fixedPoint(val)
				
				#Handle biases
				if index == "bias":
					#print("\t\tBias for node %i: %f (%i)" % (int(node), val, valFixed))
					ann[currentLayer]["Biases"][node] = valFixed #Insert bias into structure
					continue
					
					
				#Handle weights between nodes
				indexFixed = int( data["ANN"][1]["nodeTranslation"][ str( index ) ] )
				
				#print("\t\tWeight from %s(%i): %f (%i)" % (index, indexFixed, val, valFixed) )
				if not indexFixed in ann[currentLayer]["Weights"]:
					ann[currentLayer]["Weights"][indexFixed] = {}
					
				ann[currentLayer]["Weights"][indexFixed][node] = valFixed



#print("ANN:")
#print(ann)


zero = 0
emptyWord = zero.to_bytes(4, byteorder='little', signed=False)
full = 4294967295
fullWord = full.to_bytes(4, byteorder='little', signed=False) #used as prefix for signed 64-bit variables

#Write binaries using the pre-structured lists
print("\nCrafting binaries...")
for currentLayer in weights:
	
	#Create binary with bias memory
	fileName = outputDir+modelName+"_"+currentLayer+"_biases.bin"
	print("\tWriting biases for layer %s to file %s..." % (currentLayer, fileName))
	with open(fileName, "wb") as binary_file:
		for node in range(len(ann[currentLayer]["Biases"])):
			value = ann[currentLayer]["Biases"][str(node)]
			valueBin = value.to_bytes(4, byteorder='little', signed=True)
			
			#Handle signed prefix in memory
			if value < 0:
				binary_file.write(fullWord)
			else:
				binary_file.write(emptyWord)
				
			binary_file.write(valueBin)
			print("\t\tBias for node %i written to binary." % node)
			
	#Create binary with weight memory	
	fileName = outputDir+modelName+"_"+currentLayer+"_weights.bin"
	print("\tWriting weights for layer %s to file %s..." % (currentLayer, fileName))
	with open(fileName, "wb") as binary_file:
		for sourceNode in range(len(ann[currentLayer]["Weights"])):
			numWeights = len( ann[currentLayer]["Weights"][sourceNode] )
			for destNode in range(numWeights):
				#print(ann[currentLayer]["Weights"][sourceNode][str(destNode)])
				value = ann[currentLayer]["Weights"][sourceNode][str(destNode)]
				valueBin = value.to_bytes(4, byteorder='little', signed=True)
				
				#Handle signed prefix in memory
				if value < 0:
					binary_file.write(fullWord)
				else:
					binary_file.write(emptyWord)
					
				binary_file.write(valueBin)
				
				
			print("\t\t%i weights from source node %i written to binary." %(numWeights, sourceNode))
			#print(ann[currentLayer]["Weights"][node])


