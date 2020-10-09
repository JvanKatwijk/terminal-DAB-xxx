#
/*
 *    Copyright (C) 2020
 *    Jan van Katwijk (J.vanKatwijk@gmail.com)
 *    Lazy Chair Computing
 *
 *    This file is part of terminal-DAB-XXX
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

#ifndef __LIME_HANDLER__
#define	__LIME_HANDLER__

#include	<atomic>
#include	<vector>
#include	<thread>
#include	"ringbuffer.h"
#include	<LimeSuite.h>
#include	"device-handler.h"

class	limeHandler: public deviceHandler {
public:
			limeHandler	(RingBuffer<std::complex<float>> *b,
	                                 int32_t	frequency,
	                                 int16_t	gain,
	                                 std::string	antenna);
			~limeHandler	(void);
	bool		restartReader	(int32_t);
	void		stopReader	(void);
        void            resetBuffer	(void);
private:
	std::atomic<bool>	running;
	std::thread		threadHandle;
	int32_t			frequency;
	int16_t			gain;
	lms_device_t		*theDevice;
	lms_name_t		antennas [10];
	RingBuffer<std::complex<float>> *_I_Buffer;
	lms_stream_meta_t 	meta;
        lms_stream_t		stream;
        void			run		(void);
};

#endif

