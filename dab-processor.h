#
/*
 *    Copyright (C) 2016, 2017
 *    Jan van Katwijk (J.vanKatwijk@gmail.com)
 *    Lazy Chair Computing
 *
 *    This file is part of the dab-cmdline-2
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
#
#ifndef	__DAB_PROCESSOR__
#define	__DAB_PROCESSOR__
/*
 *
 */
#include	"dab-constants.h"
#include	<thread>
#include	<atomic>
#include	<stdint.h>
#include	<vector>
#include	"dab-params.h"
#include	"phasereference.h"
#include	"ofdm-decoder.h"
#include	"fic-handler.h"
#include	"msc-handler.h"
#include	"ringbuffer.h"
#include	"dab-api.h"
#include	"sample-reader.h"
//

class dabProcessor {
public:
		dabProcessor  	(RingBuffer<std::complex<float>> *,
	                         uint8_t,		// Mode
	                         syncsignal_t,
	                         systemdata_t,
	                         ensemblename_t,
	                         programname_t,
	                         fib_quality_t,
	                         audioOut_t,
	                         dataOut_t,
	                         programdata_t,
	                         programQuality_t,
	                         motdata_t,
	                         void	*);
	virtual ~dabProcessor	(void);
	void	reset			(void);
	void	stop			(void);
	void	setOffset		(int32_t);
	void	start			(void);
	bool	signalSeemsGood		(void);
	void	show_Corrector		(int);
//      inheriting from our delegates
	bool		is_audioService		(std::string);
	void		dataforAudioService     (std::string,   audiodata *);
	void		set_audioChannel        (audiodata *);
	std::string	get_ensembleName        (void);
	void		clearEnsemble           (void);
	void		reset_msc		(void);
private:
//
	RingBuffer<std::complex<float>>	*_I_Buffer;
	dabParams	params;
	sampleReader	myReader;
	phaseReference	phaseSynchronizer;
	ofdmDecoder	my_ofdmDecoder;
	ficHandler	my_ficHandler;
	mscHandler	my_mscHandler;
	syncsignal_t	syncsignalHandler;
	systemdata_t	systemdataHandler;
	programdata_t	programdataHandler;
	void		call_systemData (bool, int16_t, int32_t);
	std::thread	threadHandle;
	void		*userData;
	std::atomic<bool>	running;
	bool		isSynced;
	int		snr;
	int32_t		T_null;
	int32_t		T_u;
	int32_t		T_s;
	int32_t		T_g;
	int32_t		T_F;
	int32_t		nrBlocks;
	int32_t		carriers;
	int32_t		carrierDiff;
	bool		wasSecond	(int16_t, dabParams *);
virtual	void		run		(void);
};
#endif

