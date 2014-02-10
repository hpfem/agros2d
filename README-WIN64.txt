0) Dependencies
0.1) Recommended setup is to have the 32-bit dependencies in \...\dependencies and the 64-bit ones in \...\dependencies-64
0.2) Visual Studio 2012 Professional


1) Needed to download | build in x64:

1.0) Qt - http://download.qt-project.org/official_releases/qt/5.2/5.2.1/qt-opensource-windows-x86-msvc2012_64_opengl-5.2.1.exe

1.1) Xerces & XSD - http://www.codesynthesis.com/products/xsd/download.xhtml

1.2) UMFPACK - tricky, one has to get MinGW & compile it in 64-bits
    - MinGW used for compiling AMD and UMFPACK: `Download 64 bit version of MinGW <http://sourceforge.net/projects/mingw/>`_.
    - after installing MinGW, add 'your-minGW-installation-directory'/bin to system PATH.
    - download latest `SuiteSparse_config <http://www.cise.ufl.edu/research/sparse/SuiteSparse_config/>`_, `AMD <http://www.cise.ufl.edu/research/sparse/amd/>`_, and `UMFPACK <http://www.cise.ufl.edu/research/sparse/umfpack/>`_ to  A SINGLE PARENT DIRECTORY (this requirement is one of UMFPACK).
    - Add the following lines at the end of file SuiteSparse_config\\SuiteSparse_config.mk:

      - CC = gcc
      - CXX = gcc
      - UMFPACK_CONFIG = -DNBLAS
      - RANLIB = echo
      - LIB = -lm
    
    - Open all files called 'Makefile' from all three directories and replace all ';' symbols in them with the Windows equivalent '&'
    - Follow the instructions in UMFPACK to compile the code with MinGW
    - Copy SuiteSparse_config\\SuiteSparse_config.h to 'include' directory
    - Copy SuiteSparse_config\\libsuitesparseconfig.a to 'lib' directory and change its extension to Windows equivalent '.lib'.
    - Copy AMD\\Include\\amd.h, AMD\\Include\\amd_internal.h, and AMD\\Lib\\libamd.a to 'include', and 'lib' dependecy directories respectively. Change the libamd.a's extension to '.lib'
    - Copy UMFPACK\\Include\\* to 'include'
    - Copy UMFPACK\\Lib\\libumfpack.a to 'lib' directory and change its extension to Windows equivalent '.lib'.
    
1.3) MUMPS
  - download MUMPS from http://mumps.enseeiht.fr/MUMPS_4.10.0.tar.gz (if the link does not work, look for 4.10 version of MUMPS)
  - download WinMUMPS utility from http://sourceforge.net/projects/winmumps/
  - download a Fortran compiler (e.g. http://software.intel.com/en-us/intel-fortran-studio-xe-evaluation-options)
  - download BLAS (Debug/Release, static/dynamic, 32-bit/64-bit as you like) from http://icl.cs.utk.edu/lapack-for-windows/lapack/index.html#libraries
  - compile with VS 2012 for x64 platform (no other settings should be needed)

1.4) ZLIB - https://code.google.com/p/zlib-win64/downloads/list

1.5) Python - http://www.python.org/ftp/python/2.7.3/python-2.7.3.amd64.msi
  - Note that python27.dll will be copied to C:\Windows\System32 by the installer
  - This is highly important for the side-by-side installation of Python (32bit) and Python (64bit)
  -- Here one has to have the .dll in each of the installation folders (read further), and HAVE THOSE IN PATH env. variable
  - Recommended way to go about side-by-side installation of Python (32bit) and Python (64bit) is to have them installed in
    C:\Python27-32\ and C:\Python27-64\, and rename the currently used version (32bit or 64bit) to C:\Python27\.
  - Copy c:\Python27\Lib\site-packages\cython.py to C:\Python27\Scripts

1.6) OpenGL
  - Copy
  - c:\Program Files (x86)\Windows Kits\8.1\Lib\winv6.3\um\x64\opengl32.lib
  - c:\Program Files (x86)\Windows Kits\8.1\Lib\winv6.3\um\x64\glu32.lib
  - c:\Program Files (x86)\Windows Kits\8.1\Lib\winv6.3\um\x64\user32.lib
  - c:\Program Files (x86)\Windows Kits\8.1\Lib\winv6.3\um\x64\gdi32.lib
  -> to \...\dependencies-64/bin

1.7) Ifconsol.lib
  - https://nees.org/tools/openfresco/export/327/trunk/WIN32/lib/IFCONSOL.LIB
  - Save to \...\dependencies-64/bin


2.0) Before build
  - PATH:
  -- C:\Python27;C:\Python27\Scripts
  -- c:\Qt\Qt5.2.1\5.2.1\msvc2012_64_opengl\bin\
  -- <PATH TO AGROS REPOSITORY>\libs\
  -- \...\dependencies-64\bin
  - Wipe all the x86 outputs from the repository

2.1) Build
  - CMake: cmake -G "Visual Studio 11 Win64" .
  - Open in Visual Studio and build.