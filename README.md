
-------------------------------------------------------------------------
terminal-DAB-xxx (t-DAB)

A terminal-based dab decoding program, with slides, channel and service selection.
-------------------------------------------------------------------------

![dab-cmdline](/terminal-dab-1.png?raw=true)

As well-known, **Qt-DAB** is a large GUI driven program with a
large amount of options and widgets.
The name is - obviously - derived from Qt, the name of the framework for
the GUI, and DAB.  
**t-DAB**, or **terminal-DAB**, is also a DAB decoder. It is kept simple
and it is meant to be run from a terminal hence the name.

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
	If more than one channel is given as parameter, these channels
	are taken as channel list, and *next* and *previous* relate
	to this list.
	Otherwise, *next* and *previous* relate to all channels in Band III

A typical cmdline is

	terminal-DAB-sdrplay -C 5B -C 8A -C 8B -C 11C -C 12C -Q

The program is compiled with support for the SDRplay, and is called
with the autogain on.
A channel list is given, containing the channels 5B, 8A, 8B, 11C and 12C
(typically the channels with data that can be received here).

It will start opening the channel 5B. The "next" channel - i.e. after
touching the "+" on the keyboard - is 8A.
The "previous" channel - i.e. after touching the "-" key on the keyboard
is then channel 12C.

If only a single channel was given as parameter, e.g.

	terminal-DAB-sdrplay -C 12C -Q

then the "next" channel - i.e. after touching the "+" on the keyboard -
would be channel 12D (and the previous channel - i.e. after touching the
"-" key of the keyboard would be channel 12B).

-------------------------------------------------------------------------
Supported devices
-------------------------------------------------------------------------

Currently, the following devices are supported

	a. Adalm Pluto

	b. RTLSDR devices (also known as DABsticks)

	c. SDRplay devices using the 2.13 device library

	d. SDRplay device, using the 3.06/7 device library

	e. AIRSpy devices

	f. Hackrf devices,

	g. limesdr device.

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

	-C Channel to be added to user defined channel list
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

for lime:

	-G
	-g number	gain
	-X antenne select

for hackrf

	-G number	lna gain
	-g number	vga gain

-------------------------------------------------------------------------
Copyrights
-------------------------------------------------------------------------
	
	Copyright (C)  2020
	Jan van Katwijk (J.vanKatwijk@gmail.com)
	Lazy Chair Computing

The terminal-DAB-xxx software is made available under the GPL-2.0. The terminal-DAB-xxx program uses a number of GPL-ed libraries, all rights gratefully acknowledged.
terminal-DAB-xxx is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
See the GNU General Public License for more details.

