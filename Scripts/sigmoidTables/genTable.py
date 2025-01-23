import math

#Configuration

#Has to be consistent in Micro-C config files, ANN parameter-binary generator, and here
bitShift = 15



def sigmoid(x):
	return 1 / (1 + math.exp(-x))
	
#Convert from floats into fixed-point representation of those (using bit-shift 15)
def fixedPoint(x):
	return int(x * math.pow(2, bitShift))
	
#Convert fixedPoint into binary 64-bit notation
def binWord(x):
	return bin( x )
	
	
	
	
print(sigmoid(1) )

print(fixedPoint(1))

print( binWord( fixedPoint( sigmoid(1)  ) ) )


zero = 0
emptyWord = zero.to_bytes(4, byteorder='little', signed=True)

with open("sigmoidTab1.bin", "wb") as binary_file:
	xIndex = -10
	stepSize = 0.01
	numSteps = 400
	
	for x in range(numSteps):
		value = sigmoid( xIndex )
		valueFixed = fixedPoint(value)
		valueBin = valueFixed.to_bytes(4, byteorder='little', signed=True)
		
		print( "f(%f) = %f (%i)" %(xIndex, value, valueFixed) )
		
		xIndex = xIndex + stepSize
		
		binary_file.write(emptyWord)
		binary_file.write(valueBin)
		
with open("sigmoidTab2.bin", "wb") as binary_file:
	xIndex = -6
	stepSize = 0.005
	numSteps = 800
	
	for x in range(numSteps):
		value = sigmoid( xIndex )
		valueFixed = fixedPoint(value)
		valueBin = valueFixed.to_bytes(4, byteorder='little', signed=True)
		
		print( "f(%f) = %f (%i)" %(xIndex, value, valueFixed) )
		
		xIndex = xIndex + stepSize
		
		binary_file.write(emptyWord)
		binary_file.write(valueBin)
		
with open("sigmoidTab3.bin", "wb") as binary_file:
	xIndex = -2
	stepSize = 0.001
	numSteps = 4000
	
	for x in range(numSteps):
		value = sigmoid( xIndex )
		valueFixed = fixedPoint(value)
		valueBin = valueFixed.to_bytes(4, byteorder='little', signed=True)
		
		print( "f(%f) = %f (%i)" %(xIndex, value, valueFixed) )
		
		xIndex = xIndex + stepSize
		
		binary_file.write(emptyWord)
		binary_file.write(valueBin)
		
with open("sigmoidTab4.bin", "wb") as binary_file:
	xIndex = 2
	stepSize = 0.005
	numSteps = 800
	
	for x in range(numSteps):
		value = sigmoid( xIndex )
		valueFixed = fixedPoint(value)
		valueBin = valueFixed.to_bytes(4, byteorder='little', signed=True)
		
		print( "f(%f) = %f (%i)" %(xIndex, value, valueFixed) )
		
		xIndex = xIndex + stepSize
		
		binary_file.write(emptyWord)
		binary_file.write(valueBin)
		
with open("sigmoidTab5.bin", "wb") as binary_file:
	xIndex = 6
	stepSize = 0.01
	numSteps = 400
	
	for x in range(numSteps):
		value = sigmoid( xIndex )
		valueFixed = fixedPoint(value)
		valueBin = valueFixed.to_bytes(4, byteorder='little', signed=True)
		
		print( "f(%f) = %f (%i)" %(xIndex, value, valueFixed) )
		
		xIndex = xIndex + stepSize
		
		binary_file.write(emptyWord)
		binary_file.write(valueBin)
