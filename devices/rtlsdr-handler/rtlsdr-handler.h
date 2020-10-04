#
/*
 *    Copyright (C) 2012 .. 2020
 *    Jan van Katwijk (J.vanKatwijk@gmail.com)
 *    Lazy Chair Computing
 *
 *    This file is part of dab-xxx-cli
 *
 *    dab-xxx-cli is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 2 of the License, or
 *    (at your option) any later version.
 *
 *    dab-xxx-cli is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with dab-xxx-cli; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef __RTLSDR_HANDLER__
#define	__RTLSDR_HANDLER__

#include        <dlfcn.h>
#include	<rtl-sdr.h>
#include	"ringbuffer.h"
#include	"device-handler.h"
#include	<thread>

class	dll_driver;
//
//	This class is a simple wrapper around the
//	rtlsdr library 
//	It does not do any processing
class	rtlsdrHandler: public deviceHandler {
public:
			rtlsdrHandler	(RingBuffer<std::complex<float>> *,
	                                 int32_t	frequency,
	                                 int16_t	ppmCorrection,
	                                 int16_t	gain,
	                                 bool		autogain,
	                                 uint16_t	deviceIndex = 0);
			~rtlsdrHandler	();
//	interface to the reader
	bool		restartReader	(int32_t	frequency);
	void		stopReader	(void);
//
//	These need to be visible for the separate usb handling thread
	RingBuffer<std::complex<float>>	*_I_Buffer;
	struct rtlsdr_dev	*device;
	int32_t		sampleCounter;
private:
	int32_t		inputRate;
	uint16_t	deviceIndex;
	bool		autogain;
	int16_t		ppmCorrection;
	std::thread	workerHandle;
	int		*gains;
	int16_t		gainsCount;
	bool		running;
	int		frequency;
};
#endif

