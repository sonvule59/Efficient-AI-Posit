[![Downloads](https://static.pepy.tech/personalized-badge/qtorch-posit?period=total&units=international_system&left_color=blue&right_color=brightgreen&left_text=Downloads)](https://pepy.tech/project/qtorch-posit)

# Qtorch+: Extending Qpytorch for Posit format and beyond (table lookup, real number sets)
#### Author: minhhn2910@github, himeshi@github
---
## Install: You can either use pip or install in developer mode for editing
### Install pip package: 

```
pip install qtorch-plus
```
If you use pip package install, in the code you should import qtoch-plus like below instead of `qtorch`: 

```
import torch 
import qtorch_plus 
from qtorch_plus.quant import posit_quantize, configurable_table_quantize, Quantizer, quantizer
from qtorch_plus import Posit
from qtorch_plus.optim import OptimLP
```

### Install in developer mode: 
```bash
git clone https://github.com/minhhn2910/QPyTorch.git
cd QPyTorch
pip install -e ./
```
Simple test if c-extension is working correctly : 
```
python test.py
```
Important: if there are errors when running test.py, please export the environment variables indicating build directory and/or CUDA_HOME, otherwise we will have permission problem in multi-user-server.
```
export TORCH_EXTENSIONS_DIR=/[your-home-folder]/torch_extension
export CUDA_HOME=/[your cuda instalation directory e.g. /usr/local/cuda-10.2] 
python test.py
```
---
### Functionality: 
* Support [Posit Format](https://posithub.org/) with round to nearest mode. 
* Scaling of value before & after conversion to/from posit is supported (Exponent bias when the scale is a  power of 2).   
For example: `value x -> x*scale -> Posit(x*scale) -> x`
* Support Tanh approximation with Posit and correction of error:  
When `x` is in a posit format with es = 0 => `Sigmoid(x) = (x XOR 0x8000) >> 2 => PositTanh(x) = 2 · Sigmoid(2x) − 1 `
* More number formats (Table lookup, log2 system ...,  and new rounding modes) are currently supported in this versions, please see the tutorial below for how to use them.
#### Currently under development and update to support more number formats and schemes.
---
### CoNGA 2023 demo:
* Demo of using table lookup to simulate a customized format on GPT-2 model for text generation : [Google Colab Demo Link](https://colab.research.google.com/drive/1bpuqhfAxcvStS-K7IDyhdhjZPP9-lZ94?usp=sharing)
* Demo of using table lookup of customized format for Super resolution : [Google Colab Demo Link](https://colab.research.google.com/drive/1mT-tBy5gpn8lassGIlYwS9q1cAW9O5ot?usp=sharing)
### Demo and tutorial: 
* Important note: Some googlecolab demos below may have problems with permission because Google restricts the number of downloads per day (we use gdown to download zipped file from google drive). When errors happen, it is recommended that you try it on the next day. Or download it and run offline on your machine with GPU. Please contact me if you encounter any problem in running any of the demo.
* Approximate Tanh Function with Posit is presented at `examples/tutorial/test_posit_func.ipynb`
* Most functionalities can be tested by using the notebooks in posit tutorials: ./examples/tutorial/
* Notebook demo training Cifar10 with vanilla Posit 8 bit: `examples/tutorial/CIFAR10_Posit_Training_Example.ipynb`
* Repository contains more sample training and inference code of Vision (include Object detection on COCO dataset) and Language models [Sample Inference and Training code](https://github.com/minhhn2910/conga2022)
* Demo of YOLO-V5 on object detection model: [Google Colab Link](https://colab.research.google.com/drive/1-HGcOtrxjfnkEDEvbP7hxr0YCcbIZ_y4?usp=sharing)
* Demo of Super Resolution Task on Posit 4 bit : [Google Colab Link](https://colab.research.google.com/drive/1fApkqdfED_YJ_k6Ut3BcscMTmTcwWN3h?usp=sharing)
* Demo of DCGAN Cifar10 training with Posit 8 bit: [Google Colab Link](https://colab.research.google.com/drive/10kquzBx5tY8B5LYaxHab3HnR2lBwhwSl?usp=sharing)
* Demo of DCGAN Lsun inference using Posit 6 bit and Approximate Tanh : [Google Colab Link](https://colab.research.google.com/drive/1jNjpRTXffF1cLhV22Zzhd7LdgaZ8K_aP?usp=sharing)
* Demo of applying posit 6 bits & 8 bits to [ALBERT](https://huggingface.co/ktrapeznikov/albert-xlarge-v2-squad-v2) for Question Answering Task: [GoogleColab Demo](https://colab.research.google.com/drive/1t2bsoQb4oI-Lind_ORzroyv8X2H78cdn?usp=sharing)  


If you find this repo useful, please cite our paper(s) listed below. 

The below shows how to use this library to train and run inference on different types of neural networks. It also provides some tips and tricks on how to train the model sucessfully using Posit-8. [Link PDF](https://www.comp.nus.edu.sg/~wongwf/papers/CONGA22.pdf)
```
@inproceedings{ho2022qtorch+,
  title={Qtorch+: Next Generation Arithmetic for Pytorch Machine Learning},
  author={Ho, Nhut-Minh and De Silva, Himeshi and Gustafson, John L and Wong, Weng-Fai},
  booktitle={Conference on Next Generation Arithmetic},
  pages={31--49},
  year={2022},
  organization={Springer}
}
```

The below explains the terms and usage of Posit's enhancements (exponent bias and tanh function).
```
@inproceedings{ho2021posit,
  title={Posit Arithmetic for the Training and Deployment of Generative Adversarial Networks},
  author={Ho, Nhut-Minh and Nguyen, Duy-Thanh and De Silva, Himeshi and Gustafson, John L and Wong, Weng-Fai and Chang, Ik Joon},
  booktitle={2021 Design, Automation \& Test in Europe Conference \& Exhibition (DATE)},
  pages={1350--1355},
  year={2021},
  organization={IEEE}
}

```

---------------------------------
### The original Qpytorch package which supports floating point and fixed point:

The original README file is in REAME.original.md

Credit to the Qpytorch team and their original publication 

```
@misc{zhang2019qpytorch,
    title={QPyTorch: A Low-Precision Arithmetic Simulation Framework},
    author={Tianyi Zhang and Zhiqiu Lin and Guandao Yang and Christopher De Sa},
    year={2019},
    eprint={1910.04540},
    archivePrefix={arXiv},
    primaryClass={cs.LG}
}
```

##### Qpytorch Team
* [Tianyi Zhang](https://scholar.google.com/citations?user=OI0HSa0AAAAJ&hl=en)
* Zhiqiu Lin
* [Guandao Yang](http://www.guandaoyang.com/)
* [Christopher De Sa](http://www.cs.cornell.edu/~cdesa/)
