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

sudo apt install libxerces-c-dev

**Build**  
cd source/build  
./build.sh

**Clean**
cd source/build  
./build.sh clean  

**Install**
cd source/build  
./build.sh install  

**Package**
cd source/build  
./build.sh package  
