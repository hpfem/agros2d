Agros2D installation 
====================
 
Compilation from source files
-----------------------------
 
Compilation under GNU/Linux
^^^^^^^^^^^^^^^^^^^^^^^^^^^

Download Source Codes
"""""""""""""""""""""

Download from the Git repository::

  git clone git://github.com/hpfem/agros2d.git

Install Requirements
""""""""""""""""""""

Agros2D have following requirements:

* `Hermes <http://www.hpfem.org/hermes>`_ ()
* `Triangle <http://www.cs.cmu.edu/~quake/triangle.html>`_ ()
* `Nokia Qt <http://www.qtsoftware.com/products>`_ ()
* `Python <http://www.python.org>`_ ()
* `dxflib <http://www.ribbonsoft.com/dxflib.html>`_ ()
* `Qwt <http://qwt.sourceforge.net>`_ ()
* `FFmpeg <http://ffmpeg.org>`_ ()
* `Google Performance Tools <http://http://code.google.com/p/gperftools/wiki/GooglePerformanceTools>`_ ()

If you are using a Debian-based system, install the required libraries with following command::

  sudo apt-get install libqt4-dev libqt4-opengl-dev qt4-dev-tools qt4-qmake g++ cython 
  libssl-dev libqwt5-qt4-dev triangle-bin python-dev python-sphinx ffmpeg libsuitesparse-dev 
  libmumps-seq-dev libsuperlu3-dev libgoogle-perftools-dev 


Documentation
"""""""""""""

::

  make html

Localization
""""""""""""

::

  lrelease ./lang/*.ts

Compilation
"""""""""""

::

  qmake agros2d.pro
  make

The binary package can be build as follows:

::

  dpkg-buildpackage -sgpg -rfakeroot

Compilation under MS Windows
^^^^^^^^^^^^^^^^^^^^^^^^^^^^

How to Build Release
""""""""""""""""""""

1. Install `Git <http://http://git-scm.com>`_ client on Windows, for example `msysGit <http://code.google.com/p/msysgit>`_

  * Run Git and run ``ssh-keygen``
  * Files with keys are created in a subfolder of the user profile, in subfolder  ".ssh". In case of Windows 7 for example C:\\Users\\username\\.ssh
  * You can use these keys. If you have already these keys created, just create a folder ".ssh" and put the necessary files (``known_hosts``, ``id_rsa.pub``, ``id_rsa``) there.

2. Download current version version of Agros2D sources - from `GitHub <http://github.com>`_: `git://github.com/hpfem/agros2d.git <git://github.com/hpfem/agros2d.git>`_.
3. Download and install `QtCreator <http://qt.nokia.com/products>`_
4. Download and install Windows SDK for Qt - Qt libraries 4.7.2 for Windows (VS 2008) (http://qt.nokia.com/downloads/windows-cpp-vs2008).
5. Install Microsoft Visual Studio C++ (Express version is enough) - MSVC compiler is necessary
6. You can install additional (optional) debugger, e.g. Debugging Tools for Windows (http://download.cnet.com/Debugging-Tools-for-Windows/3000-2086_4-10907878.html)
7. You need a necessary files from "triangle"  - copy binaries from Agros binaries for Windows, copy into the project folder with Agros2D sources
8. You need a necessary files from "ffmpeg" - copy binaries from Agros binaries for Windows, copy into the project folder with Agros2D sources
9. Install `Python <http://www.python.org>`_ and `NumPy <http://numpy.scipy.org>`_

  * Copy files from Python directory into the directory of Agros2D sources especially
    
    - ``Python27\python.exe``
    - ``Python27\pythonw.exe``
    - ``Python27\libs\python27.lib``
    - Lib directory
  * Copy file ``python27.dll`` from Windows\\System32 directory into the project Agros2D source directory

10. Library "pthread" - download and extract the binary version for Windows (ftp://sourceware.org/pub/pthreads-win32/prebuilt-dll-2-8-0-release/) and from directory pthreads\\Pre-built.2\\ copy into to the project Agros2D file ``pthreadVCE2.dll``

  * into include path of Agros2D project copy following files
    
    - ``pthreads\Pre-built.2\include\semaphore.h``
    - ``pthreads\Pre-built.2\include\pthread.h``
    - ``pthreads\Pre- built.2\include\sched.h``
  * copy file ``pthreadVCE2.lib`` (pthread\\lib) into library path and rename file to ``libpthreadVCE2.lib``

11. Qwt - download form http://qwt.sourceforge.net and then compile. Use Agros2D project directory like a target directory for compilation (or optionally copy compiled files into the Agros2D project directory.
12. umfpack - necessary version is version 5.4 - download (http://www.cise.ufl.edu/research/sparse/umfpac), compile and then copy compiled files:

  * ``libumfpack.lib``, ``liamd.lib`` into Agros2D\\Lib directory (or into library path)
  * ``libumfpack.dll`` from directory UMFPACK\\Lib\\  into Agros2D directory (or into library path)
  * header files from directories AMD, UFconfig, UMFPACK copy into include path

13. Install Python http://www.python.org
14. To run debugger - add to Windows ``Path`` variable a path to the bin folder of the Qt SDK, e.g. C:\\Qt\\4.7.2\\bin
15. Copy the file ``inttypes.h`` from Hermes\\CMakeVars\\MSVC to your include directory.
16. Set in the project include path to the folder with header files (.h) files and with the libraries (.lib, .dll)
17. A problem in during compilation process of Agros2D may appears in file ``agros2d\hermes2d\src\function\solution.cpp`` - scalar method ``Solution::get_pt_value(double x, double y, int item)`` returns ``NAN`` - it has been declared in incorrectly in ``\agros2d\hermes_common\comp\c99_functions.h`` respectively ``\Agros2d\hermes_common\comp\c99_functions.cpp``. Temporary solution is to locally use for compilation correct version of these files from Hermes project's GitHub (`https://github.com/hpfem/Hermes/tree/master/hermes_common/compat <https://github.com/hpfem/Hermes/tree/master/hermes_common/compat>`_).
18. Build successful...

Building Html Documentation
"""""""""""""""""""""""""""

1. Python is necessary to have installed.
2. Install setuptools for Python (easy_install_setup, http://pypi.python.org/pypi/setuptools#windows).
3. To the system variable "path" add path PythonXX\\Scripts, e.g. C:\\Python27\\Scripts.
4. Install Sphinx documentation tool: on the command line run command ``easy_install sphinx``. It automatically download and install all necessary tools.
5. To start building of the html documentation, run on the command line in the Agros2D project folder command: ``sphinx-build -b html .\\doc\\source .\\doc\\web``
6. For convenient using is a better way to prepare windows script e.g. build_help_win.cmd including following contents:

::

	@echo off
	:: For windows platform development
	:: Windows script for html help building
	set helpSourcePath=".\\doc\\source"
	set webPath=".\\doc\\web"
	echo Building Agros2D html help...
	sphinx-build -b html %helpSourcePath% %webPath%
	echo Finished.

7. The builded html documentation can be found in the folder .\\doc\\web (in Agros2D project folder).
