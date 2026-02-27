
=======
libgz4d
=======

A collection of useful C++ classes for building interactive visualization
applications.


Building
========

For ease of maintaining cross platform versions, SCons is used to manage the
building of libgz4d.

Linux and OSX
-------------

Assuming python and scons are available, simply use scons.


To build the library, as well as test apps::

 scons

or

 scons release=1


To clean the project::

 scons -c

and

 scons -c release=1


Windows
-------

libgz4d can be built at the command line the same way as on Linux as long as
python, scons and a suitable compiler can be found on the path.

Python
~~~~~~

Python Enthought edition is used for development and testing under windows.
This python distribution includes many useful modules not included with just
python.

http://code.enthought.com/enthon/

Installing SCons
~~~~~~~~~~~~~~~~

The directory extras\win32\ contains the install packages for scons.
Install it if not already available on your system.

Visual Studio
~~~~~~~~~~~~~

A Visual Studio Solution is included that uses scons to build the project. To
allow python and scons to be found, add the python directory to the list of
directories containing executables.

  * Select **Options** from the **Tools** menu.
  * Select the **VC++ Directories** under the **Projects** folder.
  * Select **Executable Files** in the **Show directories for** drop down list.
  * Add an entry with the python directory (c:\Python24)


Command Line Build
~~~~~~~~~~~~~~~~~~

If building at the command line, the PATH envirenment variable must
contain python's installation directory (ex: c:\Python24). This can be done
the following way on WindowsXP.

  * Right-click on **My Computer** and select **Properties**
  * Select the **Advanced** tab.
  * Click the **Environment Variables** button.
  * Select the **Path** variable from the **System Variables** list.
  * Click the **Edit** button to modify it.
  * Add the python directory (;c:\Python24)at the end of the **variable value*
    if it is not already there.

Using libgz4d
=============

More to come....