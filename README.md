### This branch reflects the progress of work of building OtmMemoryService on Ubuntu OS. Description below specifies the details.

**Prerequisites**

*Operating system:*  
&nbsp;&nbsp;&nbsp;&nbsp;Ubuntu 20.10

*Compiler:*  
&nbsp;&nbsp;&nbsp;&nbsp;g++ 10.2.0

*CMake:*  
&nbsp;&nbsp;&nbsp;&nbsp;3.16.3

*Dependencies:*  
&nbsp;&nbsp;&nbsp;&nbsp;restbed:  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;https://github.com/corvusoft/restbed 
&nbsp;&nbsp;&nbsp;&nbsp;Clone the repo into the Packages dir, and build it.

**Build**  
cd source/plugins/OtmMemoryService  
mkdir build  
cmake ..  
make  

**Status notes**  
If OpenTM2 and Win API calls and data types are commented in OtmMemoryService files, executable can be built (obviously without OtmMemoryServiceGUI.CPP and WinService.CPP). 
