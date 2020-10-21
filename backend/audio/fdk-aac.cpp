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
 *
 *	Use the fdk-aac library.
 */
#include	"mp4processor.h"
//
#include	<cstring>
#include	"charsets.h"
#include	"fdk-aac.h"

//
/**
  *	\class mp4Processor is the main handler for the aac frames
  *	the class proper processes input and extracts the aac frames
  *	that are processed by the "faadDecoder" class
  */
	fdkAAC::fdkAAC (parameters *the_parameters,
	                RingBuffer<std::complex<int16_t>> *pcmBuffer,
	                void *ctx) {
	this	-> the_parameters	= the_parameters;
	this	-> pcmBuffer		= pcmBuffer;
	this	-> userData	= ctx;
	working			= false;
	handle			= aacDecoder_Open (TT_MP4_LOAS, 1);
	if (handle == nullptr)
	   return;
	working			= true;
}

	fdkAAC::~fdkAAC () {
	if (working)
	   aacDecoder_Close (handle);
}

int16_t	fdkAAC::MP42PCM (stream_parms *sp,
                         uint8_t   packet [],
                         int16_t   packetLength) {
uint32_t	packet_size;
uint32_t	valid;
AAC_DECODER_ERROR err;
uint8_t		*ptr	= packet;
INT_PCM 	decode_buf [8 * sizeof (INT_PCM) * 2048];
INT_PCM		*bufp	= &decode_buf [0];
int		output_size	= 8 * 2048; 

	if (!working)
	   return -1;

	if ((packet [0] != 0x56)  || ((packet [1] >> 5) != 7)) 
	   return -1;


	packet_size  = (((packet [1] & 0x1F) << 8) | packet [2]) + 3;
	if (packet_size != (uint32_t)packetLength)
	   return -1;

	valid = packet_size;
	err = aacDecoder_Fill (handle, &ptr, &packet_size, &valid);
	if (err != AAC_DEC_OK) 
	   return -1;

	err = aacDecoder_DecodeFrame (handle,
	                              bufp,
		                      output_size, 0);
	if (err == AAC_DEC_NOT_ENOUGH_BITS)
	   return -1;

	if (err != AAC_DEC_OK) 
	   return -1;

	CStreamInfo *info = aacDecoder_GetStreamInfo (handle);
	if (!info || info -> sampleRate <= 0) 
	   return -1;

        if (info -> numChannels == 2) {
	   output (bufp, info -> frameSize * 2,
	           sp -> aacChannelMode || sp -> psFlag, info -> sampleRate);
        }
        else
        if (info -> numChannels == 1) {
           int16_t *buffer = (int16_t *)alloca (2 * info -> frameSize);
           int16_t i;
           for (i = 0; i < info -> frameSize; i ++) {
              buffer [2 * i]	= ((int16_t *)bufp) [i];
              buffer [2 * i + 1] = bufp [2 * i];
           }
	   output (buffer, info -> frameSize * 2, false,
	                                      info -> sampleRate);
        }
        else
           fprintf (stderr, "Cannot handle these channels\n");

        return info -> numChannels; 
}

void	fdkAAC::output (int16_t *buffer, int     size,
	                bool    isStereo, int     rate) {
	if (the_parameters -> audioOutHandler == NULL)
	   return;
	the_parameters -> audioOutHandler (buffer, size, rate, isStereo, userData);
	pcmBuffer	-> putDataIntoBuffer ((std::complex<int16_t> *)buffer,
	                                                       size / 2);
}

