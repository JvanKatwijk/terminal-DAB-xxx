
-------------------------------------------------------------------------
dab-xxx-cli:
A dab decoding program for the terminal, with slides and selections.
-------------------------------------------------------------------------

![dab-cmdline](/dab-cli-curses.png?raw=true)
![dab-cmdline](/dab-xxx-cli-2.png?raw=true)

As well-known, Qt-DAB is a large GUI driven program with a
large amount of options and widgets.
At the other end of the spectrum, there is dab-cmdline with its example 2,
a  straight forward command line driven program where, once the
program is started, **everything**  - device, channel and even service -
is frozen.

For day to day use, the example 2 version from the dab-cmdline repository
is too limited, stopping and restarting the program for changing the selected
service does not make much sense.
dab-xxx-cli is a first step towards a program for the terminal handling this.
While the selected device is chosen in the configuration to build
and executable, and the channel is passed as parameter to the program

	a. once started, it shows the services from the ensemble in
	   the selected channel on the terminal. 
	   The selected service is marked by a "*".
	   Up- and down arrows (with a return as acknowledgment)
	   can be used to select a service or to change from one
	   service to another.

	b. it obviously shows the dynamic label, but on a separate widget
	   it shows slides that are transmitted as part of the service.
	   (as shown on the picture above).

------------------------------------------------------------------------
Command line parameters
------------------------------------------------------------------------

Since it is a command line version, some parameters have to be
given. One gets a list by starting the program without any parameter.

Normal use is

	dab-xxx-cli -C XXX -P YYY -G ZZZ -Q 

where

	a. -C XXX is the channel one wants to listen to

	b. -P YYY is a string representing a reasonable prefix of the servicename
	c. -G ZZZ, where Z is a valid gain value, which obviously depends on the device,

	d. -Q is the autogain setting

For other parameters, see the output of the program

If no service name is selected, the
first element of the (alfabetically sorted) list is taken.

-------------------------------------------------------------------------
Supported devices
-------------------------------------------------------------------------

Currently, the following devices are supported

	a. Adalm Pluto

	b. RTLSDR devices (also known as DABsticks)

	c. SDRplay devices using the 2.13 device library

	d. SDRplay device, using the 3.06/7 device library

	e. AIRSpy devices

---------------------------------------------------------------------------
Loading required libraries
--------------------------------------------------------------------------

Load the libraries, e.g. for Debian (Ubuntu) systems


	sudo apt-get update
	sudo apt-get install git cmake
	sudo apt-get install build-essential g++
	sudo apt-get install pkg-config
	sudo apt-get install libsndfile1-dev
	sudo apt-get install libfftw3-dev
	sudo apt-get instakk portaudio19-dev 
	sudo apt-get install zlib1g-dev 
	sudo apt-get install libusb-1.0-0-dev
	sudo apt-get install libsamplerate0-dev

For showing slides one has to install

	sudo apt-get install opencv-dev

For runnig the curses version one has to install

	sudo apt-get install curses

While the past years the faad library was used for the transformation of the
AAC data to PCM samples. It seems that the version of the faad library
that is default in the repositories of Ubuntu 20 (and may be other
distributions) is incompatible with the AAC.

On older Ubuntu systems, one might load

	sudo apt-get install libfaad-dev

It is certainly possible to download - or create - the faad-2.8 library
that has been in use for years, however, there is an easier
solution:
For the dab-xxx-cli program one may choose to use the fdkaac library
instead.

Install the library on Ubuntu

	sudo apt-get install libfdk-aac-dev

------------------------------------------------------------------------
Building an executable
------------------------------------------------------------------------

Of course, the support library for the device of choice need to
be installed as well.

if all libraries are installed, creating a makefile is by calling 
cmake with the right parameters

	mkdir build
	cd build
	cmake .. -DXXX=ON [-DYYY=OFF] [-DZZZ=OFF]

A device name should be passed as parameter, e.g. one of
	-DRTLSDR=ON 
	-DSDRPLAY=ON.
	-DSDRPLAY-V3=ON
	-DAIRSPY=ON
	-DPLUTO=ON

To let the software know that fdk-aac is the AAC decoder of choice, use

	-DFDK_LIB=ON

The software will then use the fdkaac library for the decoding of the AAC 
frames.

By default, the dab-xxx-cli with curses support is selected,
use

	-DCURSES=OFF

to deselect.

By default, the dab-xxx-cli with opencv is selected. Use

	-DPICTURES=OFF

to deselect.

When compiling on/for Linux on an X64, one might use

	-DX64_DEFINED=ON

as additional parameter. The option allows generation of 
archtecture-specific  code for the deconvolution.

The result is an executable dab-XXX-cli, where XXX is the name
of the device.

------------------------------------------------------------------------
The last step
------------------------------------------------------------------------

The last step is merely calling "make" for the generated makefile(s)

	make
	sudo make install


------------------------------------------------------------------------
A non-curses version
------------------------------------------------------------------------

Alternatively, an executable can be created that does not use the curses
library.

![dab-cmdline](/dab-cmdline.png?raw=true)

use

	-DCURSES=OFF

for this version.

------------------------------------------------------------------------
About the name of the program
------------------------------------------------------------------------

Qt-DAB is, as the name suggests, a DAB decoder using the Qt library,
dab-cmdline is, as the name suggests, something to do with dab from the
command line.
Here the  name

	dab-xxx-cli

is chosen, where the xxx is replaced by the name of the device selected
in the configuration, i.e.

	dab-pluto-cli

-------------------------------------------------------------------------
Copyrights
-------------------------------------------------------------------------
	
	Copyright (C)  2020
	Jan van Katwijk (J.vanKatwijk@gmail.com)
	Lazy Chair Computing

The dab-xxx-cli software is made available under the GPL-2.0. The dab-cmdline program uses a number of GPL-ed libraries, all rights gratefully acknowledged.
dab-cmdline is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
See the GNU General Public License for more details.

