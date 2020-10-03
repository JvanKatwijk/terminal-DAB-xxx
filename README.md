
-------------------------------------------------------------------------
dab-xxx-cli:
two command line versions of a dab decoder  (with slides and service selection)
-------------------------------------------------------------------------

![dab-cmdline](/dab-cli-curses.png?raw=true)
![dab-cmdline](/dab-cmdline.png?raw=true)

For quite some time, the "dab-cmdline" library and example programs are
functioning. While originally set up as a library feature,
the most basic example, example 2 was used most.
This is no surprise, it implements just a simple
command line driven dab decoder.

The approach taken in example 2, just starting the program with a frozen
selection of one single service is somewhat limited in practive.
While it is reasonable to stay for some time within the same channel,
the omission of the ability to select another service was hindering the use.

Two versions - merely differencing in the "main" control program
are here.

	a. one version similar to example 5 of the "dab-cmdline" examples,
	   i.e. a command line version, with the ability to select the
	   "next" and "previous" service from the list
	   (use "+" for next, and "-" for previous, followed by a return);

	b. the second version goes one step further.
	   Using the curses library the list of services remains visible.
	   In the list the currently playing service
	   is marked and with the "up" and "down" arrows one can move
	   the selection along the list (an acknowledgment with the
	   return or space key instructs the software to start the
	   new selection).

While doing so, I also wanted to see the station slide(s) on my screen,
therefore, new is a configuration option to implement showing
the slide(s), transmitted as part of the service, on the screen. 

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

If running the "regular" version, specifying a service name is required,
if running the "curses" version, if no service name is specified, the
first element of the (alfabetically sorted) list is taken.

------------------------------------------------------------------------
faad2.8 and Ubuntu 20.04
------------------------------------------------------------------------

While the past years the faad library was used for the transformation of the
AAC data to PCM samples. It seems that the version of the faad library
that is default in the repositories of Ubuntu 20 (and may be other
distributions) is incompatible with the AAC.

It is certainly possible to download - or create - the faad-2.8 library
that has been in use for years, however, there is an easier
solution:
For the dab-xxx-cli program one may choose to use the fdkaac library
instead.

Install the library on Ubuntu

	sudo apt-get install libfdk-aac-dev

Use as cmake option

	-DFDK_LIB=ON

The software will then use the fdkaac library for the decoding of the AAC 
frames.

-------------------------------------------------------------------------
Supported devices
-------------------------------------------------------------------------

Currently, the following devices are supported

	a. Adalm Pluto

	b. RTLSDR devices (also known as DABsticks)

	c. SDRplay devices (using the 2.13 device library)

	d. AIRSpy devices

---------------------------------------------------------------------------
Building an executable
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
	sudo apt-get install libfaad-dev

Note that Ubuntu 20.04 repositories provide as default an incompatible libfaad,
libfaad-2.8 is the correct version.

For showing slides one has to install

	sudo apt-get install opencv-dev

For runnig the curses version one has to install

	sudo apt-get install curses

Of course, the support library for the device of choice need to
be installed as well.

if all libraries are installed, the process is simple,

	mkdir build
	cd build
	cmake .. -DXXX=ON [-DYYY=OFF] [-DZZZ=OFF]
	make
	sudo make install

A device name should be passed as parameter, e.g. -DRTLSDR=ON or
-DSDRPLAY=ON.

The result is an executable dab-XXX-cli, where XXX is the name
of the device.

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

-------------------------------------------------------------------------
Copyrights
-------------------------------------------------------------------------
	
	Copyright (C)  2020
	Jan van Katwijk (J.vanKatwijk@gmail.com)
	Lazy Chair Computing

The dab-cmdline software is made available under the GPL-2.0. The dab-cmdline program uses a number of GPL-ed libraries, all rights gratefully acknowledged.
dab-cmdline is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
See the GNU General Public License for more details.

