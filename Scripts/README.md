# NFP NeuralNet - Scripts
This directory contains various scripts I wrote for this project, and for working with the NIC.


## Directory Structure

[sigmoidTables](sigmoidTables/) contains a script to prepare the sigmoid function lookup, when using the sigmoid activation function.

[parameters](parameters/) contains scripts to convert pre-trained neural network parameters into memory binaries that are loadable into the NIC memory.
The NN was trained with [weka](https://weka.sourceforge.io/doc.dev/weka/classifiers/functions/MultilayerPerceptron.html), and the scripts expect the input file to be in this format. See the directory for example input- and output files.

[expLookupTable](expLookupTable/) generates C code for the fixed-point lookup table.

[generateANNParameters](generateANNParameters/) is an older, and simpler, way to load neural network parameters into the NIC. This method simply generates a C file directly based on the expected parameter values. This is however deprecated, and does not support live model retraining.

[NIC](NIC/) contains various small scripts that helped me work with the NIC, such as compilation, loading, and setup.