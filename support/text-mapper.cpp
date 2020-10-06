#
/*
 *    Copyright (C) 2020
 *    Jan van Katwijk (J.vanKatwijk@gmail.com)
 *    Lazy Chair Computing
 *
 *    This file is part of terminal-DAB-xxx
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

#include	"text-mapper.h"
static 
const char *table12 [] = {
"none",
"News",
"Current Affairs",
"Information",
"Sport",
"Education",
"Drama",
"Arts",
"Science",
"Talk",
"Pop Music",
"Rock Music",
"Easy Listening",
"Light Classical",
"Classical Music",
"Other Music",
"Weather",
"Finance",
"Children\'s",
"Factual",
"Religion",
"Phone In",
"Travel",
"Leisure",
"Jazz and Blues",
"Country Music",
"National Music",
"Oldies Music",
"Folk Music",
"Documentary",
"entry 30 not used",
"entry 31 not used"
};

std::string get_programm_type_string (int16_t type) {
	if (type > 0x40) {
	   fprintf (stderr, "GUI: program type wrong (%d)\n", type);
	   return std::string (table12 [0]);
	}
	if (type < 0)
	   return std::string (" ");

	return std::string (table12 [type]);
}

static
const char *table9 [] = {
"unknown language",
"Albanian",
"Breton",
"Catalan",
"Croatian",
"Welsh",
"Czech",
"Danish",
"German",
"English",
"Spanish",
"Esperanto",
"Estonian",
"Basque",
"Faroese",
"French",
"Frisian",
"Irish",
"Gaelic",
"Galician",
"Icelandic",
"Italian",
"Lappish",
"Latin",
"Latvian",
"Luxembourgian",
"Lithuanian",
"Hungarian",
"Maltese",
"Dutch",
"Norwegian",
"Occitan",
"Polish",
"Portuguese",
"Romanian",
"Romansh",
"Serbian",
"Slovak",
"Slovene",
"Finnish",
"Swedish",
"Turkish",
"Flemish",
"Walloon",
"rfu",
"rfu",
"rfu",
"rfu",
"Reserved for national assignment",
"Reserved for national assignment",
"Reserved for national assignment",
"Reserved for national assignment",
"Reserved for national assignment",
"Reserved for national assignment",
"Reserved for national assignment",
"Reserved for national assignment",
"Reserved for national assignment",
"Reserved for national assignment",
"Reserved for national assignment",
"Reserved for national assignment",
"Reserved for national assignment",
"Reserved for national assignment",
"Reserved for national assignment",
"Reserved for national assignment"
};

static
const char *table10 [] = {
"Background sound/clean feed",
"rfu",
"rfu",
"rfu",
"rfu",
"Zulu",
"Vietnamese",
"Uzbek",
"Urdu",
"Ukranian",
"Thai",
"Telugu",
"Tatar",
"Tamil",
"Tadzhik",
"Swahili",
"Sranan Tongo",
"Somali",
"Sinhalese",
"Shona",
"Serbo-Croat",
"Rusyn",
"Russian",
"Quechua",
"Pushtu",
"Punjabi",
"Persian",
"Papiamento",
"Oriya",
"Nepali",
"Ndebele",
"Marathi",
"Moldavian",
"Malaysian",
"Malagasay",
"Macedonian",
"Laotian",
"Korean",
"Khmer",
"Kazakh",
"Kannada",
"Japanese",
"Indonesian",
"Hindi",
"Hebrew",
"Hausa",
"Gurani",
"Gujurati",
"Greek",
"Greek",
"Georgian",
"Fulani",
"Dari",
"Chuvash",
"Chinese",
"Burmese",
"Bulgarian",
"Bengali",
"Belorussian",
"Bambora",
"Azerbaijani",
"Assamese",
"Armenian",
"Arabic",
"Amharic"
};

std::string get_programm_language_string (int16_t language) {
	if (language < 0)
	   return std::string (" ");
	else if (language < 0x40)
	   return std::string (table9 [language]);
	else if (language < 0x7d)
	   return std::string (table10 [language - 0x40]);
	return std::string (table9[0]);
}

