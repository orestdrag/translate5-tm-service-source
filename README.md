### This is a version of the project successfully built on Windows 7 OS. Description below specifies the prerequisites.

** Prerequisites **

Operating system:  
    Windows 7 64-bit

Compiler:  
    Microsoft Visual Studio 2010 Professional

Windows SDK:  
    SDK v7.0A

Installer:  
    Nullsoft Installer:  
        http://nsis.sourceforge.net/Download
    FindProcDLL plugin:  
        https://nsis.sourceforge.io/FindProcDLL_plug-in#Links

Dependencies:  
    axis2/c:  
        https://axis.apache.org/axis2/c/core/download.cgi
    icu:  
        https://github.com/unicode-org/icu/releases/tag/release-50-2
    hunspell:  
        http://downloads.sourceforge.net/hunspell/hunspell-1.3.2.tar.gz

It is also necessary to install Java Development Kit, Doxygen and Python (in this case version 2.7 was used). Make sure that javac, doxygen and python paths are set in the Path environment variable.

** Adjustments **

my-config.bat file should be edited so that specified paths were accommodated for the system.

** Build **

Using Visual Studio 2010 Command Prompt run:  
    1. setvar.bat  
    2. vsvars32.bat [located at C:\Program Files (x86)\Microsoft Visual Studio 10.0\Common7\Tools]  
    3. m.bat master deb  
