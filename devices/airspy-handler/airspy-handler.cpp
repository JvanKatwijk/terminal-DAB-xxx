
/**
 *  IW0HDV Extio
 *
 *  Copyright 2015 by Andrea Montefusco IW0HDV
 *
 *  Licensed under GNU General Public License 3.0 or later. 
 *  Some rights reserved. See COPYING, AUTHORS.
 *
 * @license GPL-3.0+ <http://spdx.org/licenses/GPL-3.0+>
 *
 *	recoding, taking parts and extending for the airspyHandler interface
 *	for the SDR-J-DAB receiver and derivatives
 *	jan van Katwijk
 *	Lazy Chair Computing
 */

#include "airspy-handler.h"

static inline
std::complex<float> cmul (std::complex<float> x, float y) {
	return std::complex<float> (real (x) * y, imag (x) * y);
}

	airspyHandler::airspyHandler (RingBuffer<std::complex<float>> * _I_B,
	                              int32_t	frequency,
	                              int16_t	ppmCorrection,
	                              int16_t	theGain,
	                              bool	biasTee):
	                                deviceHandler (_I_B) {
int	result, i;
int	distance	= 10000000;
uint32_t samplerate_count;

	this	-> _I_Buffer	= _I_B;
	this	-> vfoFrequency	= frequency;
	this	-> ppmCorrection = ppmCorrection;
	this	-> theGain	= theGain;
//
	device			= 0;
	serialNumber		= 0;
//
	strcpy (serial,"");
	result	= airspy_init ();
	if (result != AIRSPY_SUCCESS) {
	   printf ("airspy_init () failed: %s (%d)\n",
	             airspy_error_name((airspy_error)result), result);
	   
	   throw (42);
	}

	fprintf (stderr, "airspy init is geslaagd\n");
	result = airspy_open (&device);
	if (result != AIRSPY_SUCCESS) {
	   printf ("airpsy_open () failed: %s (%d)\n",
	             airspy_error_name ((airspy_error)result), result);
	   throw (43);
	}

	(void) airspy_set_sample_type (device, AIRSPY_SAMPLE_INT16_IQ);
	(void) airspy_get_samplerates (device, &samplerate_count, 0);
	{
	   uint32_t	myBuffer [samplerate_count];
	   airspy_get_samplerates (device, myBuffer, samplerate_count);

	   selectedRate	= 0;
	   for (i = 0; i < (int)samplerate_count; i ++) {
	      if (abs ((int)myBuffer [i] - 2048000) < distance) {
	         distance	= abs ((int)myBuffer [i] - 2048000);
	         selectedRate = myBuffer [i];
	     }
	   }
	}

	if (selectedRate == 0) {
	   fprintf (stderr, "Sorry. cannot help you\n");
	   throw (44);
	}

	fprintf (stderr, "selected samplerate = %d\n", selectedRate);

	result = airspy_set_samplerate (device, selectedRate);
	if (result != AIRSPY_SUCCESS) {
           printf ("airspy_set_samplerate() failed: %s (%d)\n",
	             airspy_error_name((enum airspy_error)result), result);
	   throw (45);
	}

//	The sizes of the mapTable and the convTable are
//	predefined and follow from the input and output rate
//	(selectedRate / 1000) vs (2048000 / 1000)
	convBufferSize		= selectedRate / 1000;
	for (i = 0; i < 2048; i ++) {
	   float inVal	= float (selectedRate / 1000);
	   mapTable_int [i] =  int (floor (i * (inVal / 2048.0)));
	   mapTable_float [i] = i * (inVal / 2048.0) - mapTable_int [i];
	}

	convIndex		= 0;
	convBuffer. resize (convBufferSize + 1);
	running		= false;
//
//	Here we set the gain and frequency

	(void)airspy_set_freq		(device, frequency);
	(void)airspy_set_sensitivity_gain (device, theGain);
	(void)airspy_set_rf_bias	(device, biasTee ? 1 : 0);
}

	airspyHandler::~airspyHandler () {
	int result = airspy_stop_rx (device);
	if (result != AIRSPY_SUCCESS) {
	   printf ("airspy_stop_rx () failed: %s (%d)\n",
	             airspy_error_name((airspy_error)result), result);

	   result = airspy_close (device);
	   if (result != AIRSPY_SUCCESS) {
	      printf ("airspy_close () failed: %s (%d)\n",
	             airspy_error_name((airspy_error)result), result);
	   }
	}

	airspy_exit ();
}

bool	airspyHandler::restartReader	(int32_t frequency) {
int	result;

	if (running)
	   return true;

	_I_Buffer	-> FlushRingBuffer ();

	this	-> vfoFrequency = frequency;
	result = airspy_set_freq (device, frequency);
	if (result != AIRSPY_SUCCESS) {
	   printf ("airspy_set_freq() failed: %s (%d)\n",
	            airspy_error_name((airspy_error)result), result);
	}

	result = airspy_set_sample_type (device, AIRSPY_SAMPLE_INT16_IQ);
	if (result != AIRSPY_SUCCESS) {
	   printf ("airspy_set_sample_type () failed: %s (%d)\n",
	            airspy_error_name ((airspy_error)result), result);
	   return false;
	}

	result = airspy_set_sensitivity_gain (device, theGain);
	
	result = airspy_start_rx (device,
	            (airspy_sample_block_cb_fn)callback, this);
	if (result != AIRSPY_SUCCESS) {
	   printf ("airspy_start_rx () failed: %s (%d)\n",
	         airspy_error_name((airspy_error)result), result);
	   return false;
	}
//
//	finally:
	bs_ = bufSize;
	bl_ = 0;
	running	= true;
 	if (airspy_is_streaming (device) == AIRSPY_TRUE) 
	   fprintf (stderr, "restarted\n");
	
	return true;
}

void	airspyHandler::stopReader (void) {

	if (!running)
	   return;
	int result = airspy_stop_rx (device);

	if (result != AIRSPY_SUCCESS ) {
	   printf ("airspy_stop_rx() failed: %s (%d)\n",
	          airspy_error_name ((airspy_error)result), result);
	} else {
	   bs_ = bl_ = 0 ;
	}
	running	= false;
}
//
//	Directly copied from the airspy extio dll from Andrea Montefusco
int airspyHandler::callback (airspy_transfer* transfer) {
airspyHandler *p;

	if (!transfer)
	   return 0;		// should not happen

	uint32_t bytes_to_write = transfer -> sample_count * sizeof (int16_t) * 2; 
	p = static_cast<airspyHandler *> (transfer -> ctx);
	uint8_t *pt_rx_buffer   = (uint8_t *)transfer -> samples;
	
	while (bytes_to_write > 0) {
	   int spaceleft = p -> bs_ - p -> bl_ ;
	   int to_copy = std::min ((int)spaceleft, (int)bytes_to_write);
	   ::memcpy (p -> buffer + p -> bl_, pt_rx_buffer, to_copy);
	   bytes_to_write -= to_copy;
	   pt_rx_buffer   += to_copy;
//
//	   bs (i.e. buffersize) in bytes
	   if (p -> bl_ == p -> bs_) {
	      p -> data_available ((void *)p -> buffer, p -> bl_);
	      p -> bl_ = 0;
	   }
	   p -> bl_ += to_copy;
	}
	return 0;
}

//	called from AIRSPY data callback
//	this method is declared in airspyHandler class
//	The buffer received from hardware contains
//	32-bit floating point IQ samples (8 bytes per sample)
//
//	recoded for the sdr-j framework
//	2*2 = 4 bytes for sample, as per AirSpy USB data stream format
//	we do the rate conversion here, read in groups of 2 * 625 samples
//	and transform them into groups of 2 * 512 samples
int 	airspyHandler::data_available (void *buf, int buf_size) {	
int16_t	*sbuf	= (int16_t *)buf;
int nSamples	= buf_size / (sizeof (int16_t) * 2);
std::complex<float> temp [2048];
int32_t  i, j;

	for (i = 0; i < nSamples; i ++) {
	   convBuffer [convIndex ++] = std::complex<float> (sbuf [2 * i] / (float)2048,
	                                           sbuf [2 * i + 1] / (float)2048);
	   if (convIndex > convBufferSize) {
	      for (j = 0; j < 2048; j ++) {
	         int16_t  inpBase	= mapTable_int [j];
	         float    inpRatio	= mapTable_float [j];
	         temp [j]	= cmul (convBuffer [inpBase + 1], inpRatio) + 
	                          cmul (convBuffer [inpBase], 1 - inpRatio);
	      }

	      _I_Buffer	-> putDataIntoBuffer (temp, 2048);
//
//	shift the sample at the end to the beginning, it is needed
//	as the starting sample for the next time
	      convBuffer [0] = convBuffer [convBufferSize];
	      convIndex = 1;
	   }
	}
	return 0;
}
//
const char *airspyHandler::getSerial (void) {
airspy_read_partid_serialno_t read_partid_serialno;
int result = airspy_board_partid_serialno_read (device,
	                                          &read_partid_serialno);
	if (result != AIRSPY_SUCCESS) {
	   printf ("failed: %s (%d)\n",
	         airspy_error_name ((airspy_error)result), result);
	   return "UNKNOWN";
	} else {
	   snprintf (serial, sizeof(serial), "%08X%08X", 
	             read_partid_serialno. serial_no [2],
	             read_partid_serialno. serial_no [3]);
	}
	return serial;
}
//
const char* airspyHandler::board_id_name (void) {
uint8_t bid;

	if (airspy_board_id_read (device, &bid) == AIRSPY_SUCCESS)
	   return airspy_board_id_name ((airspy_board_id)bid);
	else
	   return "UNKNOWN";
}
//
