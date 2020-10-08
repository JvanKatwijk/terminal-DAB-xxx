
-------------------------------------------------------------------------
terminal-DAB-xxx

A terminal-based dab decoding program, with slides, channel and service selection.
-------------------------------------------------------------------------

![dab-cmdline](/terminal-dab-1.png?raw=true)

As well-known, **Qt-DAB** is a large GUI driven program with a
large amount of options and widgets.
**terminal-DAB** is - different from Qt-DAB - a **terminal-based**  program.
The name should be read as **terminal-DAB-devicename**, the instances
of the program have "xxx" replaced by the device name

**terminal-DAB** uses the curses library for displaying the information
on the terminal (see the picture).
The name of the channel, the name of the ensemble, the currently playing
service and the time - derived from the DAB stream are displayed
on the top of the terminal, the dynamic label - if any - is
displayed at the bottom of the terminal.

![dab-cmdline](/terminal-dab-2.png?raw=true)

If **PICTURES**  is configured (which is the default setting), slides,
that are encoded in the **Program Associated Data** part of the DAB data,
are made visible in a separate widget.

The program supports:

	selecting a service and scanning though the list of
	services. The "current" service is marked by an asterisks.
	The up- and down arrows can be used to scan through the list.

	After a channel change - and after the start up, the first
	service in the service list is marked. Select it by typing
	return.

	Selecting "next" or "previous" channel is by using the "+" resp.
	the "-" key.
	There are two options, a user defined list of - apparently -
	interesting channels can be passed as command line parameter.
	If this list is given, the "next" and "previous" channel are
	taken from this list, otherwise, they will be taken from
	the channels in Band III.

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
	sudo apt-get install curses

For showing slides one has to install

	sudo apt-get install opencv-dev

--------------------------------------------------------------------------
faad vs fdk-aac
--------------------------------------------------------------------------

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

	sudo apt-get install libfdk-aac-dev

In the CMakeLists.txt file, the default is set to libfdk-aac,
Use

	-DFAAD=ON

as option for cmake to select the faad library instead.

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

As said, use

	-DFAAD=ON

if the faad library is the AAC decoder of choice.

By default, the terminal-DAB-xxx with opencv is selected. Use

	-DPICTURES=OFF

to deselect.

When compiling on/for Linux on an X64, one might use

	-DX64_DEFINED=ON

as additional parameter. The option allows generation of 
archtecture-specific  code for the deconvolution.

The result is an executable terminal-DAB-xxx, where XXX is the name
of the device.

------------------------------------------------------------------------
The last step
------------------------------------------------------------------------

The last step is merely calling "make" for the generated makefile(s)

	make
	sudo make install

------------------------------------------------------------------------
Command line parameters
------------------------------------------------------------------------

Since it is a command line version, settings are done as parameter
in the command line.
One gets a list of "options" by starting the program without any parameter.

Normal use is

	dab-xxx-cli -C XXX -B YY1 -B YY2 ... -B YYn -G -Q

dab-cmdline options are

	-C Channel
	-B channel to be added to user defined channel list
	-A name	 select the audio channel (portaudio)

for pluto:

	-G Gain in dB (range 0 .. 70)
	-Q autogain (default off)

for rtlsdr:

	-G Gain in dB (range 0 .. 100)
	-Q autogain (default off)

for SDRplay (both versions):

	-G Gain reduction in dB (range 20 .. 59)
	-L lnaState (depends on model chosen)
	-Q autogain (default off)

for airspy:

	-G number	gain, range 1 .. 21
	-b set rf bias

-------------------------------------------------------------------------
Copyrights
-------------------------------------------------------------------------
	
	Copyright (C)  2020
	Jan van Katwijk (J.vanKatwijk@gmail.com)
	Lazy Chair Computing

The dab-xxx-cli software is made available under the GPL-2.0. The terminal-DAB-xxx program uses a number of GPL-ed libraries, all rights gratefully acknowledged.
terminal-DAB-xxx is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
See the GNU General Public License for more details.

