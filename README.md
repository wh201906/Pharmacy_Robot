# Pharmacy_Robot

for Intel's ESDC 2020  
The OpenCV is from the OpenVINO toolkit

***

It's a torture for me to write this project since I have no experience about AI and Computer Vision.  

I tried to migrate the face recognition demo into this project first. But I found it too difficult to do because I knew neither how the Inference Engine works nor the input/output of a model at that time.  
Plus, the manufacturer of the servos didn't provide the SDK on the Linux platform, so I had to sniff the communication between the Windows client and the servo controller, which cost me a lot of time.  
The edge-detection module was written by my teammate, who played an important role in this project. We called the [OCR](https://github.com/ouyanghuiyu/chineseocr_lite) directly in the program. We tried many ways to transfer the arguments between C++ and Python. Due to the lack of related knowledge, I use files to pass the frame to the OCR and get the texts. I know it sucks, but this is the only way we can find while the deadline is coming.  

I guess I'm going to explore AI and CV sooner or later, and I might find a better solution to call a Python method in a C++ program with the arguments. But now, I need to archive this as a mark of my coding experience.