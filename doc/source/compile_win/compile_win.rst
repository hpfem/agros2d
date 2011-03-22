Agros2D Compiling under Windows - how to
========================================

Build Release
-------------

* Install Git client on Windows, for example msysGit: http://code.google.com/p/msysgit
  - Run Git and run ssh-keygen
  - Files with keys are created in a subfolder of the user profile, in subfolder  “.ssh“. In case of Windows 7 for example C:\Users\username\.ssh
  - You can use these keys. If you have already these keys created, just create a folder .ssh and put the necessary files (known_hosts, id_rsa.pub, id_rsa) there.
* Download current version version of Agros2D sources - from GitHub: git://github.com/hpfem/agros2d.git
*	Download and install QtCreator - http://qt.nokia.com/products
*	Download and install Windows SDK for Qt - Qt libraries 4.7.2 for Windows (VS 2008) (http://qt.nokia.com/downloads/windows-cpp-vs2008)
*	Install Microsoft Visual Studio C++ (Express is enough) – MSVC compiler is necessary.
*	You can install additional (optional) debugger, e.g. Debugging Tools for Windows - 
http://download.cnet.com/Debugging-Tools-for-Windows/3000-2086_4-10907878.html
* Necessary files from “triangle”  - copy binaries from Agros binaries for Windows, copy into the project folder with Agros2D sources.
* Necessary files from “ffmpeg” - copy binaries from Agros binaries for Windows, copy into the project folder with Agros2D sources.
* Install Python and NumPy
  - Copy files from Python directory into the directory of Agros2D sources (http://www.python.org/download/) especially
    - file python.exe
    - file pythonw.exe
    - file Python27/libs/python27.lib
    - Lib directory
  - Copy file python27.dll from Windows\System32 directory into the project Agros2D source directory
* Library “pthread” - download and extract the binarz version for Windows (ftp://sourceware.org/pub/pthreads-win32/prebuilt-dll-2-8-0-release/) and from directory pthreads\Pre-built.2\  copy into to the project Agros2D file pthreadVCE2.dll
  - into include path of Agros2D project copy files
    - pthreads\Pre-built.2\include\semaphore.h
    - pthreads\Pre-built.2\include\pthread.h
    - pthreads\Pre- built.2\include\sched.h
  - copy file pthreadVCE2.lib (pthread\lib) into library path and rename file to libpthreadVCE2.lib
* Qwt - download form http://qwt.sourceforge.net and then compile. Use Agros2D project directory like a target directory for compilation (or optionally copy compiled files into the Agros2D project directory
* umfpack – necessary version is version 5.4 - download (http://www.cise.ufl.edu/research/sparse/umfpac),compile and then copy compiled files
  - libumfpack.lib, liamd.lib into Agros2D/Lib directory (or into library path)
  - libumfpack.dll from directory UMFPACK\Lib\  into Agros2D directory (or into library path)
  - header files from directories AMD, UFconfig, UMFPACK copy into include path
* Install Python http://www.python.org
* For debugger runnig - add to Windows PATH variable path to the bin folder of the Qt SDK, such as C:\Qt\4.7.2\bin
* Copy the file inttypes.h from Hermes/CMakeVars/MSVC to your include directory.
* Set in the project include path to the folder with header files (.h) files and with the libraries (.lib, .dll)
* A problem in during compilation process of Agros2D may appears in file agros2d\hermes2d\src\function\solution.cpp - scalar method Solution::get_pt_value(double x, double y, int item) returns NAN - it has been declared in incorrectly in \agros2d\hermes_common\comp\c99_functions.h respectively \ Agros2d\hermes_common\comp\c99_functions.cpp. Temporary solution is to locally use for compilation correct version of these files from Hermes project’s GitHub (https://github.com/hpfem/Hermes/tree/master/hermes_common/comp)
* Build successful...
