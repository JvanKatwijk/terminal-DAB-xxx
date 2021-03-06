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
#include	"dab-processor.h"
#include	"device-handler.h"
#include	"timesyncer.h"
#include	"dab-api.h"

/**
  *	\brief dabProcessor
  *	The dabProcessor class is the driver of the processing
  *	of the samplestream.
  */

	dabProcessor::dabProcessor	(RingBuffer<std::complex<float>> *_I_Buffer,
	                                 RingBuffer<std::complex<int16_t>> *pcmBuffer,
	                                 parameters	*the_parameters,
	                                 void		*userData):
	                                    params (the_parameters -> Mode),
	                                    myReader (_I_Buffer),
	                                    phaseSynchronizer (the_parameters,
	                                                       DIFF_LENGTH),
	                                    my_ofdmDecoder (the_parameters),
	                                    my_ficHandler (the_parameters,
	                                                   userData),
	                                    my_mscHandler  (the_parameters,
	                                                    pcmBuffer,
                                                            userData) {
	this	-> the_parameters	= the_parameters;
	this	-> userData		= userData;
	this	-> T_null		= params. get_T_null ();
	this	-> T_s			= params. get_T_s ();
	this	-> T_u			= params. get_T_u ();
	this	-> T_g			= params. get_T_g();
	this	-> T_F			= params. get_T_F ();
	this	-> nrBlocks		= params. get_L ();
	this	-> carriers		= params. get_carriers ();
	this	-> carrierDiff		= params. get_carrierDiff ();
	isSynced			= false;
	snr				= 0;
	running. store (false);
}

	dabProcessor::~dabProcessor	() {
	stop ();
}

void	dabProcessor::start	(void) {
	if (running. load ())
	   return;
	threadHandle	= std::thread (&dabProcessor::run, this);
}

void	dabProcessor::run	(void) {
std::complex<float>	FreqCorr;
timeSyncer      myTimeSyncer (&myReader);
int32_t		i;
float		fineOffset		= 0;
float		coarseOffset		= 0;
bool		correctionNeeded	= true;
std::vector<complex<float>>	ofdmBuffer (T_null);
int		dip_attempts		= 0;
int		index_attempts		= 0;
int		startIndex		= -1;

	isSynced	= false;
	snr		= 0;
	running. store (true);
	my_ficHandler. reset ();
	myReader. setRunning (true);
	my_mscHandler. start ();

	try {
	   myReader. reset ();
	   for (i = 0; i < T_F / 2; i ++) {
	      jan_abs (myReader. getSample (0));
	   }

notSynced:
//Initing:
           switch (myTimeSyncer. sync (T_null, T_F)) {
              case TIMESYNC_ESTABLISHED:
                 break;                 // yes, we are ready

              case NO_DIP_FOUND:
                 if  (++ dip_attempts >= 5) {
                    the_parameters -> signalHandler (false, userData);
                    dip_attempts = 0;
                 }
                 goto notSynced;

              default:                  // does not happen
              case NO_END_OF_DIP_FOUND:
                 goto notSynced;
           }

	   myReader. getSamples (ofdmBuffer. data (),
	                         T_u, coarseOffset + fineOffset);

	   startIndex = phaseSynchronizer.
	                        findIndex (ofdmBuffer. data (), THRESHOLD);
	   if (startIndex < 0) { // no sync, try again
	      isSynced	= false;
	      if (++index_attempts > 10) {
	         the_parameters -> signalHandler (false, userData);
	         index_attempts	= 0;
	      }
//	      fprintf (stderr, "startIndex %d\n", startIndex);
	      goto notSynced;
	   }

	   index_attempts	= 0;
	   goto SyncOnPhase;

Check_endofNull:
//	when we are here, we had a (more or less) decent frame,
//	and we are ready for the new one.
//	we just check that we are around the end of the null period

	   myReader. getSamples (ofdmBuffer. data (),
	                         T_u, coarseOffset + fineOffset);
	   startIndex = phaseSynchronizer.
	                       findIndex (ofdmBuffer. data (), 4 * THRESHOLD);
	   if (startIndex < 0) { // no sync, try again
	      isSynced	= false;
	      goto notSynced;
	   }

SyncOnPhase:
	   index_attempts	= 0;
	   dip_attempts		= 0;
	   isSynced		= true;
	   the_parameters -> signalHandler (isSynced, userData);

//	Once here, we are synchronized, we need to copy the data we
//	used for synchronization for block 0

	   memmove (ofdmBuffer. data (),
	            &((ofdmBuffer. data ()) [startIndex]),
	                  (T_u - startIndex) * sizeof (std::complex<float>));
	   int ofdmBufferIndex	= T_u - startIndex;

//	Block 0 is special in that it is used for coarse time synchronization
//	and its content is used as a reference for decoding the
//	first datablock.
//	We read the missing samples in the ofdm buffer
	   myReader. getSamples (&((ofdmBuffer. data ()) [ofdmBufferIndex]),
	                  T_u - ofdmBufferIndex,
	                  coarseOffset + fineOffset);
	   my_ofdmDecoder. processBlock_0	(ofdmBuffer. data ());
	   my_mscHandler.  process_mscBlock	(ofdmBuffer. data (), 0);
//
//	if correction is needed (known by the fic handler)
//	we compute the coarse offset in the phaseSynchronizer
	   correctionNeeded = !my_ficHandler. syncReached ();
	   if (correctionNeeded) {
	      int correction  = phaseSynchronizer.
	                                  estimateOffset (ofdmBuffer. data ());
	      if (correction != 100) {
	         coarseOffset += correction * carrierDiff;
	         if (abs (coarseOffset) > Khz (35))
	            coarseOffset = 0;
	      }
	   }
//
//	after block 0, we will just read in the other (params -> L - 1) blocks
//	The first ones are the FIC blocks. We immediately
//	start with building up an average of the phase difference
//	between the samples in the cyclic prefix and the
//	corresponding samples in the datapart.
///	and similar for the (params. L - 4) MSC blocks
	   FreqCorr		= std::complex<float> (0, 0);
	   std::vector<int16_t> ibits (2 * params. get_carriers ());
	   for (int ofdmSymbolCount = 1;
	        ofdmSymbolCount < (uint16_t)nrBlocks; ofdmSymbolCount ++) {	
	      myReader. getSamples (ofdmBuffer. data (),
	                               T_s, coarseOffset + fineOffset);
	      for (i = (int)T_u; i < (int)T_s; i ++) 
	         FreqCorr += ofdmBuffer [i] * conj (ofdmBuffer [i - T_u]);
//
//	Note that only the first few blocks are handled locally
//	The FIC/FIB handling is in this thread, so that there is
//	no delay is "knowing" that we are synchronized
	      if (ofdmSymbolCount < 4) {
	         my_ofdmDecoder. decode (ofdmBuffer. data (),
	                                 ofdmSymbolCount, ibits. data ());
	         my_ficHandler. process_ficBlock (ibits, ofdmSymbolCount);
	      }
	      my_mscHandler. process_mscBlock (&((ofdmBuffer. data ()) [T_g]),
	                                                   ofdmSymbolCount);
	   }

//	we integrate the newly found frequency error with the
//	existing frequency error.
	   fineOffset += 0.1 * arg (FreqCorr) / M_PI * (carrierDiff);

//	at the end of the frame, just skip Tnull samples
	   myReader. getSamples (ofdmBuffer. data (),
	                         T_null, coarseOffset + fineOffset);
	   float sum	= 0;
	   for (i = 0; i < T_null; i ++)
	      sum += abs (ofdmBuffer [i]);
	   sum /= T_null;

	   float sum2 = myReader. get_sLevel ();
	   snr	= 0.9 * snr + 0.1 * 20 * log10 ((sum2 + 0.005) / sum);

	   if (fineOffset > carrierDiff / 2) {
	      coarseOffset += carrierDiff;
	      fineOffset -= carrierDiff;
	   }
	   else
	   if (fineOffset < - carrierDiff / 2) {
	      coarseOffset -= carrierDiff;
	      fineOffset += carrierDiff;
	   }
	   goto Check_endofNull;
	}
	
	catch (int e) {
//	   fprintf (stderr, "dab processor will stop\n");
	}

	my_mscHandler.  stop ();
//	fprintf (stderr, "dabProcessor is shutting down\n");
}

void	dabProcessor:: reset	(void) {
	stop  ();
	start ();
}

void	dabProcessor::stop	(void) {	
	if (running. load ()) {
	   running. store (false);
	   myReader. setRunning (false);
	   sleep (1);
	   threadHandle. join ();
	}
}

bool	dabProcessor::signalSeemsGood	(void) {
	return isSynced;
}

bool	dabProcessor::is_audioService		(std::string serviceName) {
audiodata ad;
	dataforAudioService (serviceName, &ad);
	return ad. defined;
}

void    dabProcessor::dataforAudioService	(std::string s,audiodata *dd) {
        my_ficHandler. dataforAudioService (s, dd, 0);
}

void    dabProcessor::reset_msc (void) {
        my_mscHandler. reset ();
}

void    dabProcessor::set_audioChannel (audiodata *d) {
	my_mscHandler. reset		();
        my_mscHandler. set_audioChannel (d);
}

void    dabProcessor::clearEnsemble     (void) {
        my_ficHandler. reset ();
}

