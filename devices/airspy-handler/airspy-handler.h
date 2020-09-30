
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
 *	recoding and taking parts for the airspyRadio interface
 *	for the DAB library
 *	jan van Katwijk
 *	Lazy Chair Computing
 */
#ifndef __AIRSPY_HANDLER__
#define	__AIRSPY_HANDLER__

#include	"ringbuffer.h"
#include	"device-handler.h"
#include	<complex>
#include	<vector>

#ifndef	__MINGW32__
#include	"libairspy/airspy.h"
#else
#include	"airspy.h"
#endif

static
const   int     EXTIO_NS        =  8192;
static
const   int     EXTIO_BASE_TYPE_SIZE = sizeof (int16_t);
static
const	int	bufSize	= EXTIO_NS * EXTIO_BASE_TYPE_SIZE * 2;

class airspyHandler: public deviceHandler {
public:

			airspyHandler		(RingBuffer<std::complex<float>> *,
	                                          int32_t, int16_t,
	                                          int16_t, bool);
			~airspyHandler		();
	bool		restartReader		(int32_t);
	void		stopReader		(void);
private:
	int16_t		ppmCorrection;
	bool		running;
const	char*		board_id_name (void);
	int32_t		selectedRate;
	std::vector<std::complex<float>> convBuffer;

	int16_t		convBufferSize;
	int16_t		convIndex;
	int16_t		mapTable_int   [4 * 512];
	float		mapTable_float [4 * 512];
	RingBuffer<std::complex<float>> *theBuffer;
	struct airspy_device* device;
	uint64_t 	serialNumber;
	char		serial[128];
    // callback buffer	
	int 		bs_;
	int		bl_;
	uint8_t		buffer [bufSize];
static
	int		callback	(airspy_transfer_t *);
	int		data_available (void *buf, int buf_size);
const	char *		getSerial (void);
};

#endif
