# NFP NeuralNet
A neural network implementation for the NFP architecture, capable of real-time per-flow inference (with small MLP models) on Netronome Agilio NICs.
This work was the main result of my Bacherlos's project in 2019, and was the foundation for my talk at EuroP4'19.

The code in this repository has been through some enhancements since the thesis and talk, and contains more functionality.

I am publishing this repository following several requests from the community. The code is uploaded as-is simply to act as a reference to assist others.

Apologies in advance for the unorganized and unpolished structure and lack of documentation.
Feel free to contact me with questions.

## Directory Structure

[Scripts](Scripts/) contains various scripts to initiate the NIC.

[Code](Code/) contains the raw code for in-NIC flow-level feature extraction and neural network inference.

## Cite As
Please cite this work as:
```
@misc{langlet2019towards,
  author = {Langlet, Jonatan},
  title = {Towards Machine Learning Inference in the Data Plane},
  year = {2019},
}
```