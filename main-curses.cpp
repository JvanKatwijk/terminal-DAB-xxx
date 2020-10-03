#
/*
 *    Copyright (C) 2020
 *    Jan van Katwijk (J.vanKatwijk@gmail.com)
 *    Lazy Chair Computing
 *
 *    This file is part of the dab-xxx-cli
 *
 *    dab-cmdline-2 is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 2 of the License, or
 *    (at your option) any later version.
 *
 *    dab-cmdline-2 is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with dab-cmdline-2; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include	<stdlib.h>
#include	<curses.h>
#include	<unistd.h>
#include	<signal.h>
#include	<getopt.h>
#include        <cstdio>
#include        <iostream>
#include	<complex>
#include	<vector>
#include	<thread>
#include	"audiosink.h"
#include	"filesink.h"
#include	"dab-api.h"
#include	"dab-processor.h"
#include	"band-handler.h"
#include	"ringbuffer.h"
#include	"device-handler.h"
#ifdef	HAVE_AIRSPY
#include	"airspy-handler.h"
#elif	HAVE_PLUTO
#include	"pluto-handler.h"
#elif	HAVE_SDRPLAY
#include	"sdrplay-handler.h"
#elif	HAVE_RTLSDR
#include	"rtlsdr-handler.h"
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
//	   newService	string
//	   newTime	string
//	   newPicture	string
	typedef struct {
	   int key;
	   std::string string;
	} message;
	std::queue<message> messageQueue;
#define	S_SERVICE_CHANGE	0100
#define	S_NEW_TIME		0101
#define	S_NEW_PICTURE		0102
//	some offsets 
#define	ENSEMBLE_ROW	4
#define	ENSEMBLE_COLUMN	4
#define	PLAYING_COLUMN	(ENSEMBLE_COLUMN + 16 + 2)
#define	SERVICE_ROW	(ENSEMBLE_ROW + 1)
#define	SERVICE_COLUMN	10
#define	DOT_COLUMN	(SERVICE_COLUMN - 2)
void    printOptions	();	// forward declaration
void	listener	();
void	printServices	();
static
int		index_currentService;
static
int		indexFor		(const std::string &s);
//
//	we deal with callbacks from different threads. So, if you extend
//	the functions, take care and add locking whenever needed
static
std::atomic<bool> run;

static
dabProcessor	*theRadio	= nullptr;

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
std::string	currentService	= "";

//int32_t		serviceIdentifier	= -1;

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

void	writeMessage (int row, int column, const char *message) {
	for (uint16_t i = 0; message [i] != 0; i ++)
	   mvaddch (row, column + i, message [i]);
	move (0, 0);
	refresh ();
}
//
//	This function is called whenever the dab engine has taken
//	some time to gather information from the FIC bloks
//	the Boolean b tells whether or not an ensemble has been
//	recognized, the names of the programs are in the 
//	ensemble
static
void	ensemblenameHandler (std::string name, int Id, void *userData) {
	writeMessage (0, 10, name. c_str ());
	ensembleRecognized. store (true);
	ensembleName = name;
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
	
std::vector<std::string> serviceNames;
#include	<bits/stdc++.h>

static
void	programnameHandler (std::string s, int SId, void *userdata) {
	for (std::vector<std::string>::iterator it = serviceNames.begin();
	             it != serviceNames. end(); ++it)
	   if (*it == s)
	      return;
	serviceNames = insert (serviceNames, s);
}

static
void	programdataHandler (audiodata *d, void *ctx) {
	(void)d; (void)ctx;
//	std::cerr << "\tstartaddress\t= " << d -> startAddr << "\n";
//	std::cerr << "\tlength\t\t= "     << d -> length << "\n";
//	std::cerr << "\tsubChId\t\t= "    << d -> subchId << "\n";
//	std::cerr << "\tprotection\t= "   << d -> protLevel << "\n";
//	std::cerr << "\tbitrate\t\t= "    << d -> bitRate << "\n";
}

//
//	The function is called from within the library with
//	a string, the so-called dynamic label
static
char	theLabel [255];
static
void	dataOut_Handler (std::string dynamicLabel, void *ctx) {
uint16_t i;
	(void)ctx;
	for (i = 0; i < dynamicLabel. size (); i ++)
	   theLabel [i] = dynamicLabel. c_str () [i]; 
	for (; i < 255; i ++)	
	   theLabel [i] = ' ';
	theLabel [80] = 0;
	writeMessage (LINES - 1, 0, theLabel);
}

#ifdef	__SHOW_PICTURES__
//	The function is called from the MOT handler, with
//	as parameters the filename where the picture is stored
//	d denotes the subtype of the picture 
//	typedef void (*motdata_t)(std::string, int, void *);
void	motdataHandler (std::string s, int d, void *ctx) {
	(void)d; (void)ctx;
	message m;
	m. key		= S_NEW_PICTURE;
	m. string	= s;
	messageQueue. push (m);
}
#else
void	motdataHandler (std::string s, int d, void *ctx) {
	(void)s; (void)d; (void)ctx;
}
#endif
//
static
void	pcmHandler (int16_t *buffer, int size, int rate,
	                              bool isStereo, void *ctx) {
static bool isStarted	= false;

	(void)isStereo;
	if (!isStarted) {
	   soundOut	-> restart ();
	   isStarted	= true;
	}
	soundOut	-> audioOut (buffer, size, rate);
}

static
void	systemData (bool flag, int16_t snr, int32_t freqOff, void *ctx) {
//	fprintf (stderr, "synced = %s, snr = %d, offset = %d\n",
//	                    flag? "on":"off", snr, freqOff);
}

static
void	fibQuality	(int16_t q, void *ctx) {
//	fprintf (stderr, "fic quality = %d\n", q);
}

static
void	mscQuality	(int16_t fe, int16_t rsE, int16_t aacE, void *ctx) {
//	fprintf (stderr, "msc quality = %d %d %d\n", fe, rsE, aacE);
}

int	main (int argc, char **argv) {
// Default values
uint8_t		theMode		= 1;
std::string	theChannel	= "11C";
uint8_t		theBand		= BAND_III;
bool		autogain	= false;
#ifdef	HAVE_WAVFILES
const char	*optionsString	= "F:P:";
std::string	fileName;
#elif	HAVE_PLUTO
int16_t		gain		= 60;
const char	*optionsString	= "T:D:d:M:B:P:A:C:G:QO:";
#elif	HAVE_RTLSDR
int16_t		gain		= 60;
int16_t		ppmOffset	= 0;
const char	*optionsString	= "T:D:d:M:B:P:A:C:G:QO:";
#elif	HAVE_SDRPLAY	
int16_t		GRdB		= 30;
int16_t		lnaState	= 4;
int16_t		ppmOffset	= 0;
const char	*optionsString	= "T:D:d:M:B:P:A:C:G:L:Qp:O:";
#elif	HAVE_AIRSPY
int16_t		gain		= 20;
bool		rf_bias		= false;
int16_t		ppmOffset	= 0;
const char	*optionsString	= "T:D:d:M:B:P:A:C:G:p:bO:";
#endif
std::string	soundChannel	= "default";
int16_t		latency		= 10;
int16_t		timeSyncTime	= 10;
int16_t		freqSyncTime	= 5;
int		opt;
struct sigaction sigact;
bandHandler	dabBand;
deviceHandler	*theDevice	= nullptr;
bool	err;
int	theDuration		= -1;	// no limit
RingBuffer<std::complex<float>> _I_Buffer (16 * 32768);
#ifdef	__SHOW_PICTURES__
std::string image_path;
Mat img;
#endif
	std::cerr << "dab_cmdline example,\n \
	                Copyright 2020 J van Katwijk, Lazy Chair Computing\n";
	timeSynced.	store (false);
	timesyncSet.	store (false);
	run.		store (false);
//	std::wcout.imbue(std::locale("en_US.utf8"));
	if (argc == 1) {
	   printOptions ();
	   exit (1);
	}

#ifdef	__SHOW_PICTURES__
//	newPicture. store (true);
//	thePicture	= "/home/jan/dab-cmdline/dab-cmdline.png";
#endif
	std::setlocale (LC_ALL, "en-US.utf8");

	fprintf (stderr, "options are %s\n", optionsString);
	while ((opt = getopt (argc, argv, optionsString)) != -1) {
	   switch (opt) {
	      case 'T':
	         theDuration	= 60 * atoi (optarg);	// minutes
	         break;
	      case 'D':
	         freqSyncTime	= atoi (optarg);
	         break;

	      case 'd':
	         timeSyncTime	= atoi (optarg);
	         break;

	      case 'M':
	         theMode	= atoi (optarg);
	         if (!((theMode == 1) || (theMode == 2) || (theMode == 4)))
	            theMode = 1; 
	         break;

	      case 'B':
	         theBand = std::string (optarg) == std::string ("L_BAND") ?
	                                                 L_BAND : BAND_III;
	         break;

	      case 'P':
	         currentService	= optarg;
	         break;

	      case 'O':
	         soundOut       = new fileSink (std::string (optarg), &err);
                 if (!err) {
                    std::cerr << "sorry, could not open file\n";
                    exit (32);
                 }
	         break;

	      case 'A':
	         soundChannel	= optarg;
	         break;

	      case 'C':
	         theChannel	= std::string (optarg);
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

	      case 'p':
	         ppmOffset	= atoi (optarg);
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

	      case 'p':
	         ppmOffset	= atoi (optarg);
	         break;

#endif
	      default:
	         fprintf (stderr, "Option %c not understood\n", opt);
	         printOptions ();
	         exit (1);
	   }
	}
//
	sigact.sa_handler = sighandler;
	sigemptyset(&sigact.sa_mask);
	sigact.sa_flags = 0;

	int32_t frequency	= dabBand. Frequency (theBand, theChannel);
	try {
#ifdef	HAVE_WAVFILES
	   theDevice	= new wavFiles	(fileName. c_str (),
	                                 &_I_Buffer, nullptr);
#elif	HAVE_SDRPLAY
	   theDevice	= new sdrplayHandler (&_I_Buffer,
	                                      frequency,
	                                      ppmOffset,
	                                      GRdB,
	                                      lnaState,
	                                      autogain,
	                                      0,
	                                      0);
#elif	HAVE_AIRSPY
	   theDevice	= new airspyHandler (&_I_Buffer,
	                                     frequency,
	                                     ppmOffset,
	                                     gain,
	                                     rf_bias);
#elif	HAVE_PLUTO
	   theDevice	= new plutoHandler	(&_I_Buffer,
	                                         frequency,
	                                         gain, autogain);
#elif	HAVE_RTLSDR
	   theDevice    = new rtlsdrHandler (&_I_Buffer,
	                                     frequency,
                                             ppmOffset,
                                             gain,
                                             autogain);
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
	                                    theMode,
	                                    syncsignalHandler,
	                                    systemData,
	                                    ensemblenameHandler,
	                                    programnameHandler,
		                            timeHandler,
	                                    fibQuality,
	                                    pcmHandler,
	                                    dataOut_Handler,
	                                    programdataHandler,
	                                    mscQuality,
	                                    motdataHandler,	// MOT in PAD
	                                    nullptr		// Ctx
	                          );
	if (theRadio == nullptr) {
	   std::cerr << "sorry, no radio available, fatal\n";
	   exit (4);
	}

	theRadio	-> start ();
	theDevice	-> restartReader (frequency);

	timesyncSet.		store (false);
	ensembleRecognized.	store (false);

	while (!timeSynced. load () && (--timeSyncTime >= 0))
	   sleep (1);

	if (!timeSynced. load ()) {
	   cerr << "There does not seem to be a DAB signal here" << endl;
	   theDevice -> stopReader ();
	   sleep (1);
	   theRadio	-> stop ();
	   delete theDevice;
	   exit (22);
	}

	initscr	();
	cbreak	();
	noecho	();
	clear	();

	writeMessage (0, 0,  "there might be a DAB signal here");

	while (!ensembleRecognized. load () &&
	                             (--freqSyncTime >= 0)) {
	   sleep (1);
	}

	if (!ensembleRecognized. load ()) {
	   writeMessage (LINES - 1, 0, "no ensemble found, fatal");
	   theDevice -> stopReader ();
	   sleep (1);
	   theRadio	-> stop ();
	   delete theDevice;
	   delete soundOut;
	   endwin ();
	   exit (22);
	}

	writeMessage (0, 0, "                                    ");
	writeMessage (0, 10, "DAB command line decoder    ");
	writeMessage (1, 1, "Use up and down arrow keys to scan throught the services");
	writeMessage (2, 1, "acknowledge selection by space or return, q is quit");
	run. store (true);
	std::thread keyboard_listener = std::thread (&listener);

	sleep (5);
	char ensembleText [80];
	sprintf (ensembleText, "Ensemble: %s", ensembleName. c_str ());
	for (int i = 5; i < COLS - 5; i ++)
	   mvaddch (3, i, '=');
	writeMessage (ENSEMBLE_ROW, ENSEMBLE_COLUMN, ensembleText);
	for (uint16_t i = 0; i < serviceNames. size (); i ++)
	   writeMessage (SERVICE_ROW + i, SERVICE_COLUMN,
	                                   serviceNames [i]. c_str ());
	sleep (2);
	if (currentService == "")
	   currentService = serviceNames [0];
	
	index_currentService = indexFor (currentService);
	writeMessage (SERVICE_ROW + index_currentService, DOT_COLUMN, "*");
	while (run. load () && theDuration != 0) {
	   char text [255];
	   sprintf (text, "now playing %s", currentService. c_str ());
	   writeMessage (ENSEMBLE_ROW, PLAYING_COLUMN, text);
	   audiodata ad;
	   theRadio -> dataforAudioService (currentService. c_str (), &ad);
	   if (!ad. defined) {
	      std::cerr << "something went wrong,  we cannot handle service " << 
	                                         currentService << "\n";
	      run. store (false);
	      theRadio	-> stop ();
	      delete theDevice;
	      exit (22);
	   }

	   currentService	= ad. serviceName;
	   theRadio	-> reset_msc ();
	   theRadio	-> set_audioChannel (&ad);

	   bool		breaker	= false;
//	This polling loop is run while the service plays
	   while (run. load () && (theDuration != 0)) {
	      if (theDuration > 0)
	         theDuration --;

	      if (!messageQueue. empty ()) {
	         message m = messageQueue. front ();
	         switch (m. key) {
	            case S_SERVICE_CHANGE:
	               currentService = m. string;
	               if (!img. empty ())
	                  destroyAllWindows ();
	               breaker = true;
	               break;
	            case S_NEW_TIME:
	               writeMessage (ENSEMBLE_ROW,
	                       PLAYING_COLUMN + 30, m. string. c_str ());
	               break;
#ifdef	__SHOW_PICTURES__
	            case S_NEW_PICTURE:
	               image_path = samples::findFile (m. string);
	               destroyAllWindows ();
	               img	= imread (image_path, IMREAD_COLOR);
	               if (!img. empty ()) {
	                  imshow (image_path, img);
	                  (void)waitKey (1000);
//	                  if (c != -1)
//	                     fprintf (stderr, "key was %d\n", c);
	               }
	               break;
	#endif
	            default:
	               break;
	         }
	         messageQueue. pop ();
	         if (breaker)
	            break;
	      }
	      else
	         sleep (1);
	   }

//	      if (serviceChange. load () != -1) {
//	         currentService = serviceNames [serviceChange. load ()];
//	         serviceChange. store (-1);
//	         if (!img. empty ())
//	            destroyAllWindows ();
//	         break;
//	      }
//	      if (newTime. load ()) {
//	         writeMessage (ENSEMBLE_ROW,
//	                       PLAYING_COLUMN + 30, the_newTime. c_str ());
//	         newTime. store (false);
//	      }
//	                  
//	   
//#ifdef	__SHOW_PICTURES__
//	      if (newPicture. load ()) {
//	         newPicture. store (false);
//	         image_path	= samples::findFile (thePicture);
//	         destroyAllWindows ();
//	         img		= imread (image_path, IMREAD_COLOR);
//	         if (!img. empty ()) {
//	            imshow (image_path, img);
//	            char c = waitKey (1000);
//	            if (c != -1)
//	               fprintf (stderr, "key was %d\n", c);
//	            if (c == '\n') 
//	              serviceChange. store (-1);
//	         }
//	         else
//	            sleep (1);
//	      }
//	      else
//#endif
//	         sleep (1);
//	   }
	}
	theRadio	-> stop ();
	theDevice	-> stopReader ();
	delete theDevice;	
	delete soundOut;
	keyboard_listener. join ();
	endwin ();
}
static
int listenerState	= 0;

void	listener	(void) {
message m;
	while (run. load ()) {
	   char t = getchar ();
	   switch (t) {
	      case 'q':
	         run.store (false);
	         listenerState = 0;
	         break;
	      case '+':
	         writeMessage (SERVICE_ROW + index_currentService,
	                                             DOT_COLUMN, " ");
	         index_currentService = (index_currentService + 1 +
	                                             serviceNames. size ()) %
	                                      serviceNames. size ();
	         writeMessage (SERVICE_ROW + index_currentService, 
	                                             DOT_COLUMN, "*");
	         listenerState = 3;
	         break;
	      case '-':
	         writeMessage (SERVICE_ROW + index_currentService,
	                                             DOT_COLUMN, " ");
	         index_currentService = (index_currentService - 1 +
	                                            serviceNames. size ()) %
	                                      serviceNames. size ();
	         writeMessage (SERVICE_ROW + index_currentService,
	                                             DOT_COLUMN, "*");
	         listenerState = 3;
	         break;
	      case 0x1b:
	         listenerState = 1;
	         break;
	      case 0x5b:
	         if (listenerState == 1)
	            listenerState = 2;
	         break;
	      case 0x41:
	         if (listenerState == 2) {
	            writeMessage (SERVICE_ROW + index_currentService,
	                                DOT_COLUMN, " ");
	            index_currentService = (index_currentService -1 +
	                                       serviceNames. size ()) %
	                                      serviceNames. size ();
	            writeMessage (SERVICE_ROW + index_currentService,
	                                DOT_COLUMN, "*");
	            listenerState = 3;
	         }
	         break;
	      case 0x42:
	         if (listenerState == 2) {
	            writeMessage (SERVICE_ROW + index_currentService,
	                                 DOT_COLUMN, " ");
	            index_currentService = (index_currentService + 1 +
	                                       serviceNames. size ()) %
	                                      serviceNames. size ();
	            writeMessage (SERVICE_ROW + index_currentService,
	                                 DOT_COLUMN, "*");
	            listenerState = 3;
	         }
	         break;
	      case 'r':
	      case ' ':
	      case '\t':
	      case 012:
	      case 015:
	         if (listenerState == 3) {
	            m. key = S_SERVICE_CHANGE;
	            m. string = serviceNames [index_currentService];
	            messageQueue. push (m);
	         }
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

int	indexFor (const std::string &s) {
	for (uint16_t index = 0; serviceNames. size (); index ++)
	   if (matches (serviceNames [index], s))
	      return index;
	return 0;
}


void    printOptions	() {
	std::cerr << 
"                          dab-cmdline options are\n"
"	                  -C Channel\n"
"	                  -P name\tprogram to be selected in the ensemble\n"
"	                  -T Duration\tstop after <Duration> seconds\n"
"	                  -D number\tamount of time to look for an ensemble\n"
"	                  -d number\tseconds to reach time sync\n"
"			  -A name\t select the audio channel (portaudio)\n"
"	                  -O fileName\t output to file <name>\n"
"	                  -M Mode\tMode is 1, 2 or 4. Default is Mode 1\n"
"	                  -B Band\tBand is either L_BAND or BAND_III (default)\n"
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
"	                  -c number\t ppm Correction\n";
}

