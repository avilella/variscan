variscan
========

1. Introduction
---------------
  **variscan** implements some of the basic algorithms of nucleotide
  variability for intra- and/or interspecific polymorphism data. It
  was designed to work together with the 'LastWave' package, but can
  also be used as stand-alone software.
  
  This file is only supposed to give a quick overview of the
  **variscan** package. An extensive documentation can be found in the 
  `doc/` folder! 


2. Requirements
---------------

  * If you want to use the graphical user interface (GUI) make sure
    you have JAVA runtime environment installed (version 1.4.2 or
    higher).
  * If you want to run the scripts that this package provides you
    will need to have PERL installed.
  * **variscan** only calculates DNA polymorphism related parameters.
    If you want to do a multiresolution analysis of this data, you
    will have to download the 'LastWave' package.
  * If you want to compile **variscan** yourself you will need a
    C compiler (preferably GCC) to be installed. The installation
    of Autotools might also prove useful.


3. Files
--------

  Here is a brief description of the contents of the different
  directories:

  /. : The root directory of **variscan**. It contains documentation
       files, as well as files used in the compiling process.

  /bin : Contains precompiled executables for different operating
         systems and processor architectures.

  /data: Contains example alignments and configuration files.

  /doc: Contains an extensive documentation of **variscan** and all
        associated programs and scripts in PDF format.

  /GUI: Contains the JAVA-based graphical user interface (GUI).

  /scripts: Contains scripts to help run **variscan** and its
            associated programs.

  /src: Contains the source code of **variscan**.

  /t: A folder solely used for testing purposes by the developers.
      Nothing interesting here...


4. Installation
---------------

  * Extract the archive to a folder of your choice. A sub-folder
    called 'variscan-2.0' will appear. It contains yet another
    folder called 'bin' which contains already compiled executables
    for different systems. These are all you need to run **variscan**.

  If you don't want to use the precompiled executables but want to
  compile yourself, here is a short description on how it works
  (Make sure you have GCC installed):

  
  Linux/UNIX/Mac OSX:

  Change to the 'variscan-2.0' folder.
  If you have Autotools installed, run the 'autogen.sh' script:

    ./autogen.sh

  This will create files needed for the 'make' process.
  After that type 'make' to compile a new binary:

    make

  A new binary called **variscan** will appear in the 'src' folder.
  
  NOTE:
  If you don't have Autotools, you can try to compile by simply
  running the 'make' command alone. The package comes with some standard
  configuration files for the 'make' process that should work on most
  systems.


  Windows:

  The 'variscan-2.0' folder contains a 'variscan.dev' file which can be
  used with the freely available Dev-C++ integrated development
  environment (http:/www.bloodshed.net/dev/).
  
  If you are using command line based development tools (as for example
  the MSYS environment) you can try to rename 'Makefile.win' to
  'Makefile' and run the 'make' command. It should work in most cases.


5. Usage
--------

  * For information on how to use **variscan**, 'LastWave', the GUI
    and much more, please read our extensive documentation in the
    'doc' folder. Chapter 6 contains an in-detail example of how
    to run the different programs.


6. Supported Operating Systems
------------------------------

Variscan is developed and maintained under GNU/Linux using ANSI/ISO C
and standard development tools (GCC, Autotools).

It should compile in Win32, preferably using GNU compilers for Win32,
as those included in the Dev-C++ package.

It should compile in Mac OSX.


7. Bugs
-------
  If you encounter bugs which are not listed here, please send a
  report to the main authors (See AUTHORS).


8. Community
------------
  If you would like to see a new feature implemented, send the
  suggestion to the main authors (See AUTHORS).


9. Thanks
---------
  The authors would like to thank all people who contributed (and are
  still contributing) in the creation of this software. Please
  complain if we forgot you... ;)

  * Marc Casas -- ideas in the design of the data structures,
    debugging and lots of other stuff

  * Juan Carlos Sanchez Del Barrio -- excellent suggestions in
    first steps of the program

  * Angel Blanco Garcia -- excellent suggestions in first steps of the
    program

  * This program uses some basic utilities of Jim Kent's C
    freelib. This library saved literally hundreds of hours of our
    time. If you happen to meet Jim Kent in person, please pay him a
    beer and send the bill to us.


10. License and Disclaimer
-------------------------
  This software is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
  FOR A PARTICULAR PURPOSE. See the file 'COPYING' for more details.

  You should have received a copy of the GNU General Public License
  along with this software; if not, write to the 

    Free Software Foundation, Inc., 59 Temple Place, Suite 390, Boston,
    MA  02111-1307  USA

  This software is provided "as is" and the author is not and cannot be made
  responsible for any damage resulting from the use of this software.


