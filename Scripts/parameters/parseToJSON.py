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


#Input filename
filepath = '20_NewP4_Norm_N1000_81Per-JL'

#This list should match the implementation on the card, where which neuron takes which feature
nodeConversion = {}
nodeConversion["dur"] = 0
nodeConversion["spkts"] = 1
nodeConversion["dpkts"] = 2
nodeConversion["sbytes"] = 3
nodeConversion["dbytes"] = 4
nodeConversion["sttl"] = 5
nodeConversion["dttl"] = 6
nodeConversion["sload"] = 7
nodeConversion["dload"] = 8
nodeConversion["swin"] = 9
nodeConversion["stcpb"] = 10
nodeConversion["dtcpb"] = 11
nodeConversion["dwin"] = 12
nodeConversion["tcprtt"] = 13
nodeConversion["synack"] = 14
nodeConversion["ackdat"] = 15
nodeConversion["smean"] = 16
nodeConversion["dmean"] = 17
nodeConversion["is_sm_ips_ports"] = 18



data = {}
data['ANN'] = []

currentLayer = "Output"
layers = {}
layers[currentLayer] = []

nodes = {}



data['ANN'].append({"Weights": layers})
data['ANN'].append({"nodeTranslation": nodeConversion})


currentNode = -1
#referencedNode = -1

#Parse input-file, and generate JSON structure
with open(filepath) as fp:
	line = fp.readline()
	cnt = 1
	while line:
		words = line.split()
		line = fp.readline()
		cnt += 1
		
		if len(words) == 0: continue #skip empty lines
		
		
		#If bias value
		if words[0] == "Threshold": 
			nodes[currentNode].append({'bias': float(words[1])}) #Add bias to node
		
		#If attribute weight
		if words[0] == "Attrib":
			nodes[currentNode].append({words[1]: float(words[2])}) #Add attribute to node
			
		#If node weight
		if words[0] == "Node":
			#sourceNodeID = nodeConversion[int(words[1])]
			nodes[currentNode].append({words[1]: float(words[2])}) #Add Node weight no node
		
		#print(words)
		
		#If this is the start of a new node object
		if words[0] == "Sigmoid":
			currentNode += 1
			nodes[currentNode] = [] #Create new node
			layers[currentLayer].append({currentNode: nodes[currentNode]}) #Add new node to hidden layer
			nodeConversion[int(words[2])] = currentNode #Add translation between absolute node ID to layer index of node
			print("Processing node %i (%i) in layer '%s'..." % ( currentNode, int(words[2]), currentLayer )) #print ID of new node
			
		#If new layer object
		if words[0] == "======Hidden":
			currentLayer = "Hidden1"
			layers[currentLayer] = [] #Create new layer
			currentNode = -1
			print("Parsing layer %s..." % currentLayer)





with open('ann.json', 'w') as outfile:
    json.dump(data, outfile)
