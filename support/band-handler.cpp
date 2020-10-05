#
/*
 *    Copyright (C) 2020
 *    Jan van Katwijk (J.vanKatwijk@gmail.com)
 *    Lazy Chair Computing
 *
 *    This file is part of the t-DAB-xxx
 *
 *    t-DAB-xxx is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 2 of the License, or
 *    (at your option) any later version.
 *
 *    t-DAB-xxx is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with t-DAB-xxx; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "band-handler.h"

struct dabFrequencies {
	const char *key;
	int fKHz;
};

static
struct dabFrequencies bandIII_frequencies [] = {
{"5A",    174928},
{"5B",    176640},
{"5C",    178352},
{"5D",    180064},
{"6A",    181936},
{"6B",    183648},
{"6C",    185360},
{"6D",    187072},
{"7A",    188928},
{"7B",    190640},
{"7C",    192352},
{"7D",    194064},
{"8A",    195936},
{"8B",    197648},
{"8C",    199360},
{"8D",    201072},
{"9A",    202928},
{"9B",    204640},
{"9C",    206352},
{"9D",    208064},
{"10A",   209936},
{"10B",   211648},
{"10C",   213360},
{"10D",   215072},
{"11A",   216928},
{"11B",   218640},
{"11C",   220352},
{"11D",   222064},
{"12A",   223936},
{"12B",   225648},
{"12C",   227360},
{"12D",   229072},
{"13A",   230748},
{"13B",   232496},
{"13C",   234208},
{"13D",   235776},
{"13E",   237488},
{"13F",   239200},
{NULL, 0}
};

static
struct dabFrequencies Lband_frequencies [] = {
{"LA", 1452960},
{"LB", 1454672},
{"LC", 1456384},
{"LD", 1458096},
{"LE", 1459808},
{"LF", 1461520},
{"LG", 1463232},
{"LH", 1464944},
{"LI", 1466656},
{"LJ", 1468368},
{"LK", 1470080},
{"LL", 1471792},
{"LM", 1473504},
{"LN", 1475216},
{"LO", 1476928},
{"LP", 1478640},
{NULL, 0}
};

bandHandler::bandHandler	(uint8_t theBand) {
	this	-> theBand	= theBand;
}

bandHandler::~bandHandler	() {
}

//    find the frequency for a given channel in a given band
int32_t bandHandler::Frequency (const std::string &Channel) {
int32_t tunedFrequency = 0;
struct dabFrequencies *finger;
int i;

	if (theBand == BAND_III)
	   finger = bandIII_frequencies;
	else
	   finger = Lband_frequencies;

	for (i = 0; finger[i].key != NULL; i++) {
	   if (finger[i].key == Channel) {
	      tunedFrequency = finger[i].fKHz * 1000;
	      break;
	   }
	}

	if (tunedFrequency == 0)
	   tunedFrequency = finger[0].fKHz * 1000;

	return tunedFrequency;
}

std::string bandHandler::nextChannel (const std::string &Channel) {
struct dabFrequencies *finger;
int i;

	if (theBand == BAND_III)
	   finger = bandIII_frequencies;
	else
	   finger = Lband_frequencies;

	for (i = 0; finger[i].key != NULL; i++) {
	   if (finger[i].key == Channel) {
	      if (finger[i + 1].key == NULL)
	         return finger[0].key;
	      else
                 return finger[i + 1].key;
	      break;
	   }
	}

	return "";
}

std::string bandHandler::prevChannel (const std::string &Channel) {
struct dabFrequencies *finger;
int	max;
int	i;
	if (theBand == BAND_III)
	   finger = bandIII_frequencies;
	else
	   finger = Lband_frequencies;

	for (i = 0; finger [i]. key != nullptr; i ++);
	max = i - 1;
	for (i = 0; finger[i].key != NULL; i++) {
	   if (finger[i].key == Channel) {
	      if (i == 0)
	         return finger[max].key;
	      else
                 return finger[i - 1].key;
	      break;
	   }
	}

	return "";
}
