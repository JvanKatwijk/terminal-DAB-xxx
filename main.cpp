#
/*
 *    Copyright (C) 2020
 *    Jan van Katwijk (J.vanKatwijk@gmail.com)
 *    Lazy Chair Computing
 *
 *    This file is part of the terminal-DAB
 *
 *    terminal-DAB is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 2 of the License, or
 *    (at your option) any later version.
 *
 *    terminal-DAB is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with terminal-DAB; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include	<stdlib.h>
#include	<unistd.h>
#include	<signal.h>
#include	<getopt.h>
#include        <iostream>
#include	<complex>
#include	<vector>
#include	"audiosink.h"
#include	"filesink.h"
#include	"dab-api.h"
#include	"dab-processor.h"
#include	"band-handler.h"
#include	"ringbuffer.h"
#include	"locking-queue.h"
#include	"display-handler.h"
#include	"device-handler.h"
#ifdef	HAVE_AIRSPY
#include	"airspy-handler.h"
#elif	HAVE_PLUTO
#include	"pluto-handler.h"
#elif	HAVE_SDRPLAY
#include	"sdrplay-handler.h"
#elif	HAVE_SDRPLAY_V3
#include	"sdrplay-handler-v3.h"
#elif	HAVE_RTLSDR
#include	"rtlsdr-handler.h"
#elif	HAVE_HACKRF
#include	"hackrf-handler.h"
#elif	HAVE_LIMESDR
#include	"lime-handler.h"
#elif	HAVE_WAVFILES
#include	"wavfiles.h"
#endif
#ifdef	__SHOW_PICTURES__
#include	<opencv2/core.hpp>
#include	<opencv2/imgcodecs.hpp>
#include	<opencv2/highgui.hpp>
using namespace cv;
#endif
#include	<locale>
#include	<codecvt>
#include	<atomic>
#include	<string>
#include	<queue>
using std::cerr;
using std::endl;

//
//	messages
typedef struct {
   int key;
   std::string string;
} message;
static
lockingQueue<message>messageQueue;
displayHandler	theDisplay;

//
//	Valid signals:
#define	S_START			0100
#define	S_QUIT			0101
#define	S_SET_NEXTCHANNEL	0102
#define	S_SET_PREVCHANNEL	0103
#define	S_SET_NEXTSERVICE	0104
#define	S_SET_PREVSERVICE	0105
#define	S_TIMER_TICK		0106
#define	S_ACKNOWLEDGE		0200
#define	S_NEW_TIME		0201
#define	S_NEW_PICTURE		0203
#define	S_NEW_AUDIO		0204
#define	S_ENSEMBLE_FOUND	0300
#define	S_SERVICE_NAME		0301
#define	S_DYNAMICLABEL		0302

void    printOptions	();	// forward declaration
void	listener	();
static
int		index_currentService;
//
//	we deal with callbacks from different threads. So, if you extend
//	the functions, take care and add locking whenever needed
static
std::atomic<bool> run;
static
deviceHandler	*theDevice	= nullptr;
static
std::string	deviceName	= "";
static
dabProcessor	*theRadio	= nullptr;

bandHandler	dabBand (BAND_III);
static
std::atomic<bool>timeSynced;

static
std::atomic<bool>timesyncSet;

static
std::atomic<bool>ensembleRecognized;

static
std::string ensembleName;

static
audioBase	*soundOut	= nullptr;

static
RingBuffer<std::complex<float>> _I_Buffer (16 * 32768);
static
RingBuffer<std::complex<int16_t>> pcmBuffer (32768);

static void sighandler (int signum) {
	fprintf (stderr, "Signal caught, terminating!\n");
	run. store (false);
}

static
void	syncsignalHandler (bool b, void *userData) {
	timeSynced. store (b);
	timesyncSet. store (true);
	(void)userData;
}

static
void	timeHandler	(std::string theTime, void *userData) {
message m;
	m. key		= S_NEW_TIME;
	m. string	= theTime;
	messageQueue. push (m);
}

static
void	ensemblenameHandler (std::string name, int Id, void *userData) {
message m;
	ensembleRecognized. store (true);
	ensembleName = name;
	m. key		= S_ENSEMBLE_FOUND;
	m. string	= name;
	messageQueue. push (m);
}

std::vector<std::string>
	 insert (std::vector<std::string> names, std::string newName) {
std::vector<std::string> res;
bool	inserted = false;
	for (uint16_t i = 0; i < names. size (); i ++) {
	   if (!inserted && (names [i]. compare (newName) > 0)) {
	      res. push_back (newName);
	      res. push_back (names [i]);
	      inserted = true;
	   }
	   else
	      res. push_back (names [i]);
	}
	if (!inserted)
	   res. push_back (newName);
	return res;
}

static
std::vector<std::string> serviceNames;

static
void	addtoEnsemble (std::string s, int SId, void *userdata) {
message m;
	(void)SId;
	m. key		= S_SERVICE_NAME;
	m. string	= s;
	messageQueue. push (m);
}

static
void	addtoServices (std::string s) {
	for (std::vector<std::string>::iterator it = serviceNames.begin();
	   it != serviceNames. end(); ++it)
	   if (*it == s)
	      return;
	serviceNames = insert (serviceNames, s);
	for (uint16_t i = 0; i < serviceNames. size (); i ++)
	   theDisplay. showService (serviceNames [i], i);
}

static
void	dynamicLabelHandler (std::string dynamicLabel, void *ctx) {
message m;
	m. key		= S_DYNAMICLABEL;
	m. string	= dynamicLabel;
	messageQueue. push (m);
}

#ifdef	__SHOW_PICTURES__
static
void	motdataHandler (std::string s, int d, void *ctx) {
	(void)d; (void)ctx;
	message m;
	m. key		= S_NEW_PICTURE;
	m. string	= s;
	messageQueue. push (m);
}
#else
void    motdataHandler (std::string s, int d, void *ctx) {
        (void)s; (void)d; (void)ctx;
}
#endif

static
void	audioOutHandler (int16_t *buffer, int size, int rate,
	                              bool isStereo, void *ctx) {
	(void)isStereo;
	message m;
        m. key          = S_NEW_AUDIO;
        m. string       = to_string (rate);
        messageQueue. push (m);
}

static	
void	pcmHandler	(int rate) {
static	bool isStarted	= false;

	if (!isStarted) {
	   soundOut	-> restart ();
	   isStarted	= true;
	}
	int16_t localBuf [1024];
	while (pcmBuffer. GetRingBufferReadAvailable () > 1024 / 2) {
	   pcmBuffer. getDataFromBuffer (localBuf, 1024 /2);
//	   soundOut	-> audioOut (localBuf, 1024, rate);
	}
}

//
//	computing the "next" channel
//
std::vector<std::string> userChannels;
static
std::string	nextChannel	(const std::string &s, bool dir) {
int size = userChannels. size ();

	if (size > 1) {
	   for (int i = 0; i < size; i ++) {
	      if (userChannels. at (i) == s) {
	         if (dir)
	            return userChannels. at ((i + 1) % size);
	         else
	            return userChannels. at ((i - 1 + size) % size);
	      }
	   }
	}
	else
	if (dir)
	   return dabBand. nextChannel (s);
	return dabBand. prevChannel (s);
}

void	selectService	(const std::string &s) {
audiodata ad;

	theRadio -> dataforAudioService (s. c_str (), &ad);
	if (!ad. defined) {
	   theDisplay. show_playing ("no data");
	   return;
	}

//	currentService       = ad. serviceName;
	theRadio     -> set_audioChannel (&ad);
	theDisplay. show_playing (ad. serviceName);
	theDisplay. show_audioData (&ad);
}

parameters	the_parameters;
std::string	theChannel	= "12C";
uint8_t		theMode		= 1;

int	main (int argc, char **argv) {
bool		autogain	= false;
#ifdef	HAVE_WAVFILES
const char	*optionsString	= "F:P:";
std::string	fileName;
#elif	HAVE_PLUTO
int16_t		gain		= 60;
const char	*optionsString	= "B:D:d:A:C:G:Q";
#elif	HAVE_RTLSDR
int16_t		gain		= 60;
int16_t		ppmOffset	= 0;
const char	*optionsString	= "B:D:d:A:C:G:Q";
#elif	HAVE_SDRPLAY	
int16_t		GRdB		= 30;
int16_t		lnaState	= 4;
int16_t		ppmOffset	= 0;
const char	*optionsString	= "B:D:d:P:A:C:G:L:Q";
#elif	HAVE_SDRPLAY_V3	
int16_t		GRdB		= 30;
int16_t		lnaState	= 4;
int16_t		ppmOffset	= 0;
const char	*optionsString	= "B:D:d:P:A:C:G:L:Q";
#elif	HAVE_AIRSPY
int16_t		gain		= 20;
bool		rf_bias		= false;
int16_t		ppmOffset	= 0;
const char	*optionsString	= "B:D:d:P:A:C:G:b";
#elif	HAVE_HACKRF
int		lnaGain		= 40;
int		vgaGain		= 40;
int		ppmOffset	= 0;
bool		ampEnable	= false;
const char	*optionsString	= "D:d:A:C:G:g:p:a";
#elif	HAVE_LIMESDR
int16_t		gain		= 70;
std::string	antenna		= "Auto";
const char	*optionsString	= "D:d:A:C:G:g:X:";
#endif
std::string	soundChannel	= "default";
int16_t		latency		= 10;
int		opt;
struct sigaction sigact;
bool	err;
#ifdef	__SHOW_PICTURES__
std::string image_path;
Mat img;
#endif

	the_parameters. Mode			= 1;
	the_parameters. signalHandler		= syncsignalHandler;
	the_parameters. timeHandler		= timeHandler;
	the_parameters. ensembleHandler		= ensemblenameHandler;
	the_parameters. audioOutHandler		= audioOutHandler;
	the_parameters. programnameHandler	= addtoEnsemble;
	the_parameters. dynamicLabelHandler	= dynamicLabelHandler;
	the_parameters. motdataHandler		= motdataHandler;

	std::cerr << "dab-xxx-cli,\n \
	                Copyright 2020 J van Katwijk, Lazy Chair Computing\n";
	timeSynced.	store (false);
	timesyncSet.	store (false);
	run.		store (false);
//	std::wcout.imbue(std::locale("en_US.utf8"));

	if (argc == 1) {
	   printOptions ();
	   exit (1);
	}

	std::setlocale (LC_ALL, "en-US.utf8");

	fprintf (stderr, "options are %s\n", optionsString);
	while ((opt = getopt (argc, argv, optionsString)) != -1) {
	   switch (opt) {
	      case 'A':
	         soundChannel	= optarg;
	         break;

	      case 'C':
	         userChannels. push_back (std::string (optarg));
	         fprintf (stderr, "%s \n", optarg);
	         break;

#ifdef	HAVE_WAVFILES
	      case 'F':
	         fileName	= std::string (optarg);
	         break;
#elif	HAVE_PLUTO
	      case 'G':
	         gain		= atoi (optarg);
	         break;

	      case 'Q':
	         autogain	= true;
	         break;

#elif	HAVE_RTLSDR
	      case 'G':
	         gain		= atoi (optarg);
	         break;

	      case 'Q':
	         autogain	= true;
	         break;

	      case 'p':
	         ppmOffset	= atoi (optarg);
	         break;

#elif	HAVE_SDRPLAY
	      case 'G':
	         GRdB		= atoi (optarg);
	         break;

	      case 'L':
	         lnaState	= atoi (optarg);
	         break;

	      case 'Q':
	         autogain	= true;
	         break;

#elif	HAVE_SDRPLAY_V3
	      case 'G':
	         GRdB		= atoi (optarg);
	         break;

	      case 'L':
	         lnaState	= atoi (optarg);
	         break;

	      case 'Q':
	         autogain	= true;
	         break;

#elif	HAVE_AIRSPY
	      case 'G':
	         gain		= atoi (optarg);
	         break;

	      case 'Q':
	         autogain	= true;
	         break;

	      case 'b':
	         rf_bias	= true;
	         break;
#elif	HAVE_HACKRF
	      case 'G':
	         lnaGain	= atoi (optarg);
	         break;

	      case 'g':
	         vgaGain	= atoi (optarg);
	         break;

	      case 'p':
	         ppmOffset	= 0;
	         break;

	      case 'a':
	         ampEnable	= true;
	         break;

#elif	HAVE_LIME
	      case 'G':
	      case 'g':	
	         gain		= atoi (optarg);
	         break;

	      case 'X':
	         antenna	= std::string (optarg);
	         break;
#endif
	      default:
	         fprintf (stderr, "Option %c not understood\n", opt);
	         printOptions ();
	         exit (1);
	   }
	}
//
	if (userChannels. size () == 0) {
	   fprintf (stderr, "please specify a channel here\n");
	   exit (21);
	}
	theChannel		= userChannels. at (0);

	sigact.sa_handler = sighandler;
	sigemptyset(&sigact.sa_mask);
	sigact.sa_flags = 0;
	int32_t frequency	= dabBand. Frequency (theChannel);
	try {
#ifdef	HAVE_SDRPLAY_V3
	   theDevice	= new sdrplayHandler_v3 (&_I_Buffer,
	                                         frequency,
	                                         ppmOffset,
	                                         GRdB,
	                                         lnaState,
	                                         autogain,
	                                         0,
	                                         0);
	deviceName	= "sdrplay";
#elif	HAVE_SDRPLAY
	   theDevice	= new sdrplayHandler (&_I_Buffer,
	                                      frequency,
	                                      ppmOffset,
	                                      GRdB,
	                                      lnaState,
	                                      autogain,
	                                      0,
	                                      0);
	deviceName	= "sdrplay";
#elif	HAVE_AIRSPY
	   theDevice	= new airspyHandler (&_I_Buffer,
	                                     frequency,
	                                     ppmOffset,
	                                     gain,
	                                     rf_bias);
	deviceName	= "airspy";
#elif	HAVE_PLUTO
	   theDevice	= new plutoHandler	(&_I_Buffer,
	                                         frequency,
	                                         gain, autogain);
	deviceName	= "pluto";
#elif	HAVE_RTLSDR
	   theDevice    = new rtlsdrHandler (&_I_Buffer,
	                                     frequency,
                                             ppmOffset,
                                             gain,
                                             autogain);
	deviceName	= "rtlsdr";
#elif	HAVE_HACKRF
	   theDevice	= new hackrfHandler	(&_I_Buffer,
	                                         frequency,
	                                         ppmOffset,
	                                         lnaGain,
	                                         vgaGain,
	                                         ampEnable);
#elif	HAVE_LIME
	   theDevice	= new limeHandler	(&_I_Buffer,
	                                         frequency, gain, antenna);
#elif	HAVE_WAVFILES
	   theDevice	= new wavFiles	(fileName. c_str (),
	                                 &_I_Buffer, nullptr);
#endif

	}
	catch (int e) {
	   std::cerr << "allocating device failed (" << e << "), fatal\n";
	   exit (32);
	}
	if (theDevice == nullptr) {
	   fprintf (stderr, "no device selected, fatal\n");
	   exit (33);
	}
//
	if (soundOut == nullptr) {
	   soundOut	= new audioSink	(latency, soundChannel, &err);
	   if (err) {
	      std::cerr << "no valid sound channel, fatal\n";
	      exit (33);
	   }
	}

//	and with a sound device we now can create a "backend"
	theRadio	= new dabProcessor (&_I_Buffer,
	                                    &pcmBuffer,
	                                    &the_parameters,
	                                    nullptr		// Ctx
	                          );
	if (theRadio == nullptr) {
	   std::cerr << "sorry, no radio available, fatal\n";
	   delete theDevice;
	   delete soundOut;
	   exit (34);
	}

//	here we start
	theDisplay. startDisplay ();
	message m;
	m. key	= S_START;
	m. string	= "";
	sleep (2);
	messageQueue. push (m);

	bool	channelChanged	= false;
	run. store (true);
	std::thread keyboard_listener = std::thread (&listener);
	std::thread timer;
	channelChanged	= true;
	while (run. load ()) {
	   message m;
	   while (!messageQueue. pop (10000, &m));

	   switch (m. key) {
	      case S_START:
	         theRadio	-> start ();
                 theDevice	-> restartReader (frequency);
	         index_currentService	= 0;
	         theDisplay. showHeader (deviceName);
	         break;

	      case S_QUIT:
	         run. store (false);
	         break;

	      case S_SET_NEXTCHANNEL:
	         theChannel	= nextChannel (theChannel, true);
#ifdef	__SHOW_PICTURES__
	         if (!img. empty ())
	            destroyAllWindows ();
#endif
	         theDevice	-> stopReader ();
	         theRadio	-> stop ();
	         ensembleRecognized. store (false);
	         sleep (1);
	         theDisplay. mark_service (index_currentService, " ");
	         theDisplay. clearServices (serviceNames. size ());
	         theDisplay. clear_audioData	();
	         theDisplay. showChannel (theChannel);
	         serviceNames. resize (0);
	         sleep (1);
	         theDevice	-> restartReader (dabBand. Frequency (theChannel));
	         theRadio	-> start ();
	         index_currentService = 0;
	         channelChanged	= true;
	         break;

	      case S_SET_PREVCHANNEL:
	         theChannel	= nextChannel (theChannel, false);
#ifdef __SHOW_PICTURES__
	         if (!img. empty ())
	            destroyAllWindows ();
#endif
	         theDevice	-> stopReader ();
	         theRadio	-> stop ();
	         ensembleRecognized. store (false);
	         sleep (1);
	         theDisplay. mark_service (index_currentService, " ");
	         theDisplay. clearServices (serviceNames. size ());
	         theDisplay. clear_audioData	();
	         theDisplay. showChannel (theChannel);
	         serviceNames. resize (0);
	         sleep (1);
	         theDevice	-> restartReader (dabBand. Frequency (theChannel));
	         theRadio	-> start ();
	         index_currentService = 0;
	         channelChanged	= true;
	         break;

	      case S_SET_NEXTSERVICE:
	         channelChanged		= false;
	         theDisplay. mark_service (index_currentService, " ");
	         theDisplay. clear_audioData ();
	         index_currentService = (index_currentService + 1) %
	                                             serviceNames. size ();
	         theDisplay. mark_service (index_currentService, "*");
#ifdef	__SHOW_PICTURES__
	         if (!img. empty ())
	            destroyAllWindows ();
#endif
	         selectService (serviceNames [index_currentService]);
	         break;

	      case S_SET_PREVSERVICE:
	         channelChanged		= false;
	         theDisplay. mark_service (index_currentService, " ");
	         theDisplay. clear_audioData ();
	         index_currentService = (index_currentService - 1 +
	                                             serviceNames. size ()) %
	                                             serviceNames. size ();
	         theDisplay. mark_service (index_currentService, "*");
#ifdef	__SHOW_PICTURES__
	         if (!img. empty ())
	            destroyAllWindows ();
#endif
	         selectService (serviceNames [index_currentService]);
	         break;

	      case S_ACKNOWLEDGE:
	         if (channelChanged) {
	           theDisplay. mark_service (index_currentService, "*");
	           selectService (serviceNames [index_currentService]);
	           break;
	         }
	         break;

	      case S_NEW_TIME:
	         theDisplay. showTime (m. string);
	         break;
	      case S_NEW_PICTURE:
#ifdef	__SHOW_PICTURES__
	         image_path = samples::findFile (m. string);
	         destroyAllWindows ();
	         img	= imread (image_path, IMREAD_COLOR);
	         if (!img. empty ()) {
	            imshow (image_path, img);
	            (void)waitKey (500);
	         }
#endif
	         break;
	      case S_NEW_AUDIO:
	         pcmHandler (std::stoi (m. string));
	         break;

	      case S_ENSEMBLE_FOUND:
	         theDisplay. showEnsemble (theChannel, ensembleName);
	         break;

	      case S_SERVICE_NAME:
	         addtoServices (m. string);
	         if (ensembleRecognized. load ()) {
	            index_currentService = 0;
	            theDisplay. mark_service (index_currentService, "*");
	         }
	         break;

	      case S_DYNAMICLABEL:
	         theDisplay. show_dynamicLabel (m. string);
	         break;

	      default:
	         break;
	   }
	}

//	termination:
	theDevice	-> stopReader ();
	theRadio	-> stop ();
	theDisplay. stop ();
	soundOut	-> stop	();
	keyboard_listener. join ();
	delete	soundOut;
	delete theDevice;
	delete theRadio;
}

static
int listenerState	= 0;

void	listener	(void) {
message m;
	while (run. load ()) {
	   char t = getchar ();
	   switch (t) {
	      case 'q':
	         m. key = S_QUIT;
	         m. string = "";
	         fprintf (stderr, "pushed Q\n");
	         messageQueue. push (m);
	         listenerState = 0;
	         break;
	      case '+':
	         m. key = S_SET_NEXTCHANNEL;
	         m. string = "";
	         messageQueue. push (m);
	         listenerState = 0;
	         break;
	      case '-':
	         m. key = S_SET_PREVCHANNEL;
	         m. string = "";
	         messageQueue. push (m);
	         listenerState = 0;
	         break;
	      case 0x1b:
	         listenerState = 1;
	         break;
	      case 0x5b:
	         if (listenerState == 1)
	            listenerState = 2;
	         break;
	      case 0x41:
	         m. key = S_SET_PREVSERVICE;
	         m. string = "";
	         messageQueue. push (m);
	         listenerState = 0;
	         break;
	      case 0x42:
	         m. key = S_SET_NEXTSERVICE;
	         m. string = "";
	         messageQueue. push (m);
	         listenerState = 0;
	         break;
	      case 'r':
	      case ' ':
	      case '\t':
	      case 012:
	      case 015:
	         m. key	= S_ACKNOWLEDGE;
	         m. string = "";
	         messageQueue. push (m);
	         listenerState	= 0;
	         break;

	      default:
	         break;
	   }
	}
}

bool	matches (std::string s1, std::string s2) {
const char *ss1 = s1. c_str ();
const char *ss2 = s2. c_str ();

	while ((*ss1 != 0) && (*ss2 != 0)) {
	   if (*ss2 != *ss1)
	      return false;
	   ss1 ++;
	   ss2 ++;
	}
	return (*ss2 == 0) && ((*ss1 == ' ') || (*ss1 == 0));
}

void    printOptions	() {
	std::cerr << 
"                          dab-cmdline options are\n"
"	                  -C Channel\n"
"	                  -B channel to be added to user defined channel list\n"
"			  -A name\t select the audio channel (portaudio)\n"
"	for pluto:\n"
"	                  -G Gain in dB (range 0 .. 70)\n"
"	                  -Q autogain (default off)\n"
"	for rtlsdr:\n"
"	                  -G Gain in dB (range 0 .. 100)\n"
"	                  -Q autogain (default off)\n"
"	                  -p number\t ppm offset\n"
"	for SDRplay:\n"
"	                  -G Gain reduction in dB (range 20 .. 59)\n"
"	                  -L lnaState (depends on model chosen)\n"
"	                  -Q autogain (default off)\n"
"	                  -p number\t ppm offset\n"
"	for airspy:\n"
"	                  -G number\t	gain, range 1 .. 21\n"
"	                  -b set rf bias\n"
"	                  -c number\t ppm Correction\n"
"       for hackrf:\n"
"                         -v vgaGain\n"
"                         -l lnaGain\n"
"                         -a amp enable (default off)\n"
"       for limesdr:\n"
"                         -G number\t gain\n"
"                         -X antenna selection\n"
"                         -C channel\n";
}

