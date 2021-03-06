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
#
#ifndef	__FIB_PROCESSOR__
#define	__FIB_PROCESSOR__
#
//
#include	<stdint.h>
#include	<stdio.h>
#include	<string>
#include	<mutex>
#include	<atomic>
#include	"dab-api.h"
#include	"dab-constants.h"

	struct dablabel {
	   std::string	label;
	   bool		hasName;
	};

	typedef struct dablabel	dabLabel;

	typedef struct subchannelmap channelMap;
//	from FIG1/2
	struct serviceid {
	   bool		inUse;
	   uint32_t	serviceId;
	   dabLabel	serviceLabel;
	   bool		hasPNum;
	   bool		hasLanguage;
	   int16_t	language;
	   int16_t	programType;
	   uint16_t	pNum;
	};

	typedef	struct serviceid serviceId;
//      The service component describes the actual service
//      It really should be a union
        struct servicecomponents {
           bool         inUse;          // just administration
           int8_t       TMid;           // the transport mode
           serviceId    *service;       // belongs to the service
           int16_t      componentNr;    // component

           int16_t      ASCTy;          // used for audio
           int16_t      PS_flag;        // use for both audio and packet
           int16_t      subchannelId;   // used in both audio and packet
           uint16_t     SCId;           // used in packet
           uint8_t      CAflag;         // used in packet (or not at all)
           int16_t      DSCTy;          // used in packet
	   uint8_t	DGflag;		// used for TDC
           int16_t      packetAddress;  // used in packet
	   int16_t	appType;	// used in packet
	   bool		is_madePublic;
        };

        typedef struct servicecomponents serviceComponent;

	struct subchannelmap {
	   bool		inUse;
	   int32_t	SubChId;
	   int32_t	StartAddr;
	   int32_t	Length;
	   bool		shortForm;
	   int32_t	protLevel;
	   int32_t	BitRate;
	   int16_t	language;
	   int16_t	FEC_scheme;
	};


class	fib_processor {
public:
		fib_processor		(parameters	*,
	                                 void		*);
		~fib_processor		(void);
	void	process_FIB		(uint8_t *, uint16_t);

	void	setupforNewFrame	(void);
	void	clearEnsemble		(void);
	bool	syncReached		(void);
	void	dataforAudioService	(std::string &, audiodata *);
	void	dataforAudioService	(std::string &, audiodata *, int16_t);

	void	reset			();

private:
	parameters	*the_parameters;
	void		*userData;
	int32_t		dateTime [8];
	serviceId	*findServiceId (int32_t);
	serviceComponent *find_serviceComponent (int32_t SId, int16_t SCId);
	serviceId	*findServiceId	(std::string);
        void            bind_audioService (int8_t,
                                           uint32_t, int16_t,
                                           int16_t, int16_t, int16_t);
	void		process_FIG0		(uint8_t *);
	void		process_FIG1		(uint8_t *);
	void		FIG0Extension0		(uint8_t *);
	void		FIG0Extension1		(uint8_t *);
	void		FIG0Extension2		(uint8_t *);
	void		FIG0Extension5		(uint8_t *);
	void		FIG0Extension6		(uint8_t *);
	void		FIG0Extension9		(uint8_t *);
	void		FIG0Extension10		(uint8_t *);
        void            FIG0Extension17         (uint8_t *);

	int16_t		HandleFIG0Extension1	(uint8_t *,
	                                         int16_t, uint8_t);
	int16_t		HandleFIG0Extension2	(uint8_t *,
	                                         int16_t, uint8_t, uint8_t);
	int16_t		HandleFIG0Extension5	(uint8_t *, int16_t);
	int16_t		HandleFIG0Extension8	(uint8_t *,
	                                         int16_t, uint8_t);
	int16_t		HandleFIG0Extension13	(uint8_t *,
	                                         int16_t, uint8_t);
	channelMap	subChannels [64];
	serviceComponent	ServiceComps [64];
	serviceId	listofServices [64];
        bool            dateFlag;
//
//	additional data for ex-10 functions
	uint8_t         ecc_byte;
        uint8_t         interTabId;
        bool            firstTime;
        bool            ecc_Present;
        bool            interTab_Present;
	std::atomic<int32_t>  CIFcount;
        std::atomic<bool>     hasCIFcount;
//	end of additionall data for ex-10 functions
	bool		isSynced;
	mutex		fibLocker;
//
//	these were signals
	void		addtoEnsemble	(const std::string &, int32_t);
	void		nameofEnsemble  (int, const std::string &);
	void		changeinConfiguration (void);
};

#endif

