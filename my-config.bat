@ECHO OFF
rem Copyright (c) 2013, International Business Machines
rem Corporation and others.  All rights reserved.

rem    File containing the individual drive and path settings
rem
rem    This file is called by setvar.bat
rem
rem    Please adjust this drive and path settings to your enviroment
rem    Please also note that the message compiler file can only process file name with 8.3 restrictions. Avoid
rem    using any directory or file name longer than 8.3.
rem
rem Drive of OpenTM2 source
set _DRIVE=C:
rem OpenTM2 source root directory
set _DEVDIR=translate5-tm-service-source
rem Drive letter of VisualStudio and Windows SDK installation
set _COMP=C:

rem tool for OS/2 format message file creation
set _MSG_MAKER=C:\TOOLS\MSMSGF.EXE
SET ZIPTOOL="C:\Program Files (x86)\7-Zip\7z.exe"
SET ZIPOPTIONS=a -r

rem Microsoft Visual C++ settings
rem    These settings have to be adjusted as well when other version of VisualStudio are being used
rem    or the programs have not been installed using the standard installation settings
set _windowssdk=C:\Program Files (x86)\Microsoft SDKs\Windows\v7.0A
set vclib=C:\Program Files (x86)\Microsoft Visual Studio 10.0\VC\lib;C:\Program Files (x86)\Microsoft Visual Studio 10.0\VC\atlmfc\lib;%_windowssdk%\Lib;
set VCINCLUDE=C:\Program Files (x86)\Microsoft Visual Studio 10.0\VC\include;%_COMP%\Program Files (x86)\Microsoft Visual Studio 10.0\VC\mfc\include;%_DRIVE%\%_DEVDIR%winincl;C:\Program Files (x86)\Microsoft Visual Studio 10.0\VC\atlmfc\include;%_windowssdk%\Include

rem settings for Axis2/C + staff package
SET AXIS2C_HOME=C:\axis2c
SET STAFF_HOME=C:\staff
SET _AXIS2C_INCL=C:\axis2c\include

rem settings for NSIS installer
SET _NSISMAKE_EXE="C:\Program Files (x86)\NSIS\makensis"

rem settings for ICU
SET _ICU_INCL=C:\icu\include
SET ICULIB=C:\icu\lib

rem settings for HUNSPELL
SET _HUNSPELL_INCL=C:\hunspell\src
SET HUNSPELLLIBDIR=%_DRIVE%\%_DEVDIR%\Packages\lib


