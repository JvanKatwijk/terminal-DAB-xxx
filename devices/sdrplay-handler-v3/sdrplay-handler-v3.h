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

#ifndef __SDRPLAY_HANDLER_V3__
#define	__SDRPLAY_HANDLER_V3__

#include        <stdint.h>
#include        <thread>
#include        <atomic>
#include        <vector>
#include	"dab-constants.h"
#include	"ringbuffer.h"
#include	"device-handler.h"
#include	<sdrplay_api.h>

class	sdrplayHandler_v3: public deviceHandler {
public:
		sdrplayHandler_v3 (RingBuffer<std::complex<float>> *b,
	                           uint32_t	frequency,
                                   int16_t	ppmCorrection,
                                   int16_t	GRdB,
                                   int16_t	lnaState,
                                   bool		autogain,
                                   uint16_t	deviceIndex,
                                   int16_t	antenna);
		~sdrplayHandler_v3	();
	bool    restartReader           (int32_t);
        void    stopReader              (void);
//	The following items should be visible from outsize
//	the callback functions refer to them
        RingBuffer<std::complex<float>> *_I_Buffer;
        float   denominator;
        void    update_PowerOverload (sdrplay_api_EventParamsT *params);
        std::atomic<bool>       running;
private:
        void                    run             ();
        sdrplay_api_DeviceT             *chosenDevice;
        sdrplay_api_DeviceParamsT       *deviceParams;
        sdrplay_api_CallbackFnsT        cbFns;
        sdrplay_api_RxChannelParamsT    *chParams;
        std::thread                     threadHandle;

        bool                    failFlag;
        int16_t                 hwVersion;
        int32_t                 vfoFrequency;
        int16_t                 ppmCorrection;
        int16_t                 GRdB;
        int16_t                 lnaState;
        bool                    autogain;
        uint16_t                deviceIndex;
	int16_t			antenna;
	int16_t			nrBits;
};
#endif

