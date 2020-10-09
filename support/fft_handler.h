#
/*
 *    Copyright (C) 2020
 *    Jan van Katwijk (J.vanKatwijk@gmail.com)
 *    Lazy Chair Computing
 *
 *    This file is part of terminal-DAB
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

#ifndef __FFT_HANDLER__
#define	__FFT_HANDLER__
//
//	Simple wrapper around fftwf
#include	"dab-constants.h"
#include	"dab-params.h"
#include	<fftw3.h>


class	fft_handler {
public:
			fft_handler	(uint8_t);
			~fft_handler	(void);
	complex<float>	*getVector	(void);
	void		do_FFT		(void);
	void		do_iFFT		(void);
private:
	dabParams	p;
	int32_t		fftSize;
	complex<float>	*vector;
	fftwf_plan	plan;
};

#endif

