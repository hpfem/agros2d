Compiling under GNU/Linux
=========================

Download Source Codes
---------------------

Download from the Git repository::

  git clone git://github.com/hpfem/agros2d.git

Install Requirements
--------------------

Agros2D have following requirements:

* `Hermes <http://www.hpfem.org/hermes>`_ ()
* `Triangle <http://www.cs.cmu.edu/~quake/triangle.html>`_ ()
* `Nokia Qt <http://www.qtsoftware.com/products>`_ ()
* `Python <http://www.python.org>`_ ()
* `dxflib <http://www.ribbonsoft.com/dxflib.html>`_ ()
* `Qwt <http://qwt.sourceforge.net>`_ ()
* `FFmpeg <http://ffmpeg.org>`_ ()

If you are using a Debian-based system, install the required libraries with following command::

  sudo apt-get install libqt4-dev libqt4-opengl-dev qt4-dev-tools qt4-qmake g++ cython libssl-dev libqwt5-qt4-dev triangle-bin python-dev python-sphinx ffmpeg libsuitesparse-dev libmumps-seq-dev libsuperlu3-dev

Documentation
-------------

::

  make html

Localization
------------

::

  lrelease ./lang/*.ts

Compilation
-----------

::

  qmake agros2d.pro
  make

The binary package can be build as follows:

::

  dpkg-buildpackage -sgpg -rfakeroot
