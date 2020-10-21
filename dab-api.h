#
/*
 *    Copyright (C) 2020
 *    Jan van Katwijk (J.vanKatwijk@gmail.com)
 *    Lazy Chair Computing
 *
 *    This file is the API description of terminal-DAB
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
 *    along with terminal-DAB, if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
#ifndef		__DAB_API__
#define		__DAB_API__
#include	<stdio.h>
#include	<stdint.h>
#include	<string>
#include	<complex>
#include	"ringbuffer.h"

//	Experimental API for controlling the dab software library
//
#include	<stdint.h>
//
//
typedef	struct {
	bool	defined;
	std::string	serviceName;
	int16_t	subchId;
	int16_t	startAddr;
	bool	shortForm;
	int16_t	protLevel;
	int16_t	length;
	int16_t	bitRate;
	int16_t	ASCTy;
	int16_t	language;
	int16_t	programType;
	bool	is_madePublic;
} audiodata;

//////////////////////// C A L L B A C K F U N C T I O N S ///////////////
//
//
//	A signal is sent as soon as the library knows that time
//	synchronization will be ok.
//	Especially, if the value sent is false, then it is (almost)
//	certain that no ensemble will be detected
	typedef void (*syncsignal_t)(bool, void *);
//
//	the ensemblename is sent whenever the library detects the
//	name of the ensemble
	typedef void (*ensemblename_t)(std::string, int32_t, void *);
//
//	Each programname in the ensemble is sent once
	typedef	void (*programname_t)(std::string, int32_t, void *);
//
//	the fib also sends the time
	typedef	void	(*theTime_t)(std::string, void *);
//
//	after selecting an audio program, the audiooutput, packed
//	as PCM data (always two channels) is sent back
	typedef void (*audioOut_t)(int16_t *,		// buffer
	                           int,			// size
	                           int,			// samplerate
	                           bool,		// stereo
	                           void *);
//
//	dynamic label data, embedded in the audio stream, is sent as string
	typedef void (*dataOut_t)(std::string, void *);
//
//	MOT pictures - i.e. slides encoded in the Program Associated data
//	are stored in a file. Each time such a file is created, the
//	function registered as
	typedef void (*motdata_t)(std::string, int, void *);
//	is invoked (if not specified as NULL)
//
typedef struct {
	uint8_t		Mode;
        syncsignal_t    signalHandler;
        theTime_t       timeHandler;
        ensemblename_t  ensembleHandler;
        programname_t   programnameHandler;
	audioOut_t	audioOutHandler;
        dataOut_t       dynamicLabelHandler;
        motdata_t       motdataHandler;
} parameters;
//
#endif
