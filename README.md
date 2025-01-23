# NFP NeuralNet
This repository provides an implementation of a neural network for the Network Flow Processor (NFP) architecture. 
This implementation enables real-time flow inference on Netronome Agilio NICs, suitable for smaller NN models.

This project was the core of my Bachelor's thesis in 2019 and served as the foundation for my talk at EuroP4'19.

I am publishing this repository following several requests from the community. 
The code is however uploaded as-is simply to act as a reference to assist others.

Apologies in advance for the unorganized and unpolished structure and lack of documentation.
Feel free to contact me with questions.

## Directory Structure

[Scripts](Scripts/) contains various scripts to initiate the NIC and neural network.

[Code](Code/) contains the raw code for in-NIC flow-level feature extraction and neural network inference.

## Cite As
If you find this work helpful, please cite it as:
```
@misc{langlet2019towards,
  author = {Langlet, Jonatan},
  title = {Towards Machine Learning Inference in the Data Plane},
  year = {2019},
}
```