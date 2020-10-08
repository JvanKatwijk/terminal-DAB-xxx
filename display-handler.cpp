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

#include	"text-mapper.h"
#include	"dab-api.h"
#include	"display-handler.h"
#include	"curses.h"

//	some offsets 
#define	ENSEMBLE_ROW	4
#define	ENSEMBLE_COLUMN	4
#define	PLAYING_COLUMN	(ENSEMBLE_COLUMN + 22 + 2)
#define	SERVICE_ROW	(ENSEMBLE_ROW + 1)
#define	SERVICE_COLUMN	10
#define	DOT_COLUMN	(SERVICE_COLUMN - 2)

#define	AUDIODATA_LINE		10
#define	AUDIODATA_COLUMN	40

	displayHandler::displayHandler	() {
}

	displayHandler::~displayHandler	() {
//	endwin ();
}

void	displayHandler::startDisplay	() {
	initscr ();
        cbreak  ();
        noecho  ();
}

void	displayHandler::stop		() {
	endwin ();
}


void	displayHandler::writeMessage	(int row,
	                                 int column, const char *message) {
        for (uint16_t i = 0; message [i] != 0; i ++)
           mvaddch (row, column + i, message [i]);
        move (0, 0);
        refresh ();
}

void	displayHandler::showTime	(const std::string &s) {
	writeMessage (ENSEMBLE_ROW, PLAYING_COLUMN + 30, s. c_str ());
}

void	displayHandler::showHeader	(const std::string &deviceName) {
std::string text	= std::string ("terminal-DAB-") + deviceName;
	writeMessage (0, 20, text. c_str ());
        writeMessage (1, 1, "Use + and - keys to scan throught the channels");
        writeMessage (2, 1, "Use up and down arrow keys to scan throught the services");
	for (int i = 5; i < COLS - 5; i ++)
	   writeMessage (3, i, "=");
}

void	displayHandler::showEnsemble	(const std::string &theChannel,
	                                 const std::string &ensembleName) {
std::string text	= theChannel + 
	                  std::string (": Ensemble: ") +
	                  ensembleName;
        writeMessage (ENSEMBLE_ROW, ENSEMBLE_COLUMN, text. c_str ());
}

void	displayHandler::showChannel	(const std::string &theChannel) {
	writeMessage (ENSEMBLE_ROW, ENSEMBLE_COLUMN, theChannel. c_str ());
	for (int i = ENSEMBLE_COLUMN + 4; i < COLS; i ++)
	   writeMessage (ENSEMBLE_ROW, i, " ");
}

void	displayHandler::showService	(const std::string &s, int index) {
	writeMessage (SERVICE_ROW + index, SERVICE_COLUMN, s. c_str ());
}

void	displayHandler::clear_audioData () {
	writeMessage (AUDIODATA_LINE + 0, AUDIODATA_COLUMN, "                ");
	writeMessage (AUDIODATA_LINE + 1, AUDIODATA_COLUMN, "                ");
	writeMessage (AUDIODATA_LINE + 2, AUDIODATA_COLUMN, "                ");
	writeMessage (AUDIODATA_LINE + 3, AUDIODATA_COLUMN, "                ");
}

void	displayHandler::clearServices	(int nrServices) {
	for (uint16_t i = 0; i < nrServices; i ++)
           writeMessage (SERVICE_ROW + i, SERVICE_COLUMN,
                                           "                ");
}

void	displayHandler::show_playing	(const std::string &s) {
std::string text	= std::string (" now playing ") + s;
	writeMessage (ENSEMBLE_ROW, PLAYING_COLUMN, text. c_str ());
}

std::string displayHandler::getProtectionLevel (bool shortForm,
	                                          int16_t protLevel) {
	if (!shortForm) {
	   switch (protLevel) {
	      case 0:     return "EEP 1-A";
	      case 1:     return "EEP 2-A";
	      case 2:     return "EEP 3-A";
	      case 3:     return "EEP 4-A";
	      case 4:     return "EEP 1-B";
	      case 5:     return "EEP 2-B";
	      case 6:     return "EEP 3-B";
	      case 7:     return "EEP 4-B";
	      default:    return "EEP unknown";
	   }
	}
	else {
	   switch (protLevel) {
	      case 1:     return "UEP 1";
	      case 2:     return "UEP 2";
	      case 3:     return "UEP 3";
	      case 4:     return "UEP 4";
	      case 5:     return "UEP 5";
	      default:    return "UEP unknown";
	   }
	}
}

static const
char *uep_rates [] = {"7/20", "2/5", "1/2", "3/5", "3/4"};
static const
char *eep_Arates[] = {"1/4",  "3/8", "1/2", "3/4"}; 
static const
char *eep_Brates[] = {"4/9",  "4/7", "4/6", "4/5"}; 

std::string displayHandler::getCodeRate (bool shortForm, int16_t protLevel) {
int h = protLevel;

        if (!shortForm)
           return ((h & (1 << 2)) == 0) ?
                            eep_Arates [h & 03] :
                            eep_Brates [h & 03]; // EEP -A/-B
        else
           return uep_rates [h - 1];     // UEP
}

void	displayHandler::show_audioData	(audiodata *ad) {
std::string bitRate	= std::string ("bitrate ") +
	                               std::to_string (ad -> bitRate);
std::string type	= ad -> ASCTy == 077 ? "DAB+" : "DAB";
std::string programType	= get_programm_type_string (ad -> programType);
std::string protLevel	= getProtectionLevel (ad -> shortForm,
	                                          ad -> protLevel);
	protLevel += std::string ("  ");
	protLevel += getCodeRate (ad -> shortForm, ad -> protLevel);

	writeMessage (AUDIODATA_LINE + 0, AUDIODATA_COLUMN, type. c_str ());
	writeMessage (AUDIODATA_LINE + 1, AUDIODATA_COLUMN, bitRate. c_str ());
	writeMessage (AUDIODATA_LINE + 2, AUDIODATA_COLUMN, programType. c_str ());
	writeMessage (AUDIODATA_LINE + 3, AUDIODATA_COLUMN, protLevel. c_str ());
}

void	displayHandler::mark_service (int index, const std::string &s) {
	writeMessage (SERVICE_ROW + index, DOT_COLUMN, s. c_str ());
}

void	displayHandler::show_dynamicLabel	(const std::string &dynLab) {
char text [COLS];
uint16_t	i;

	for (i = 0; (i < dynLab. size ()) && (i < COLS - 1); i ++)
	   text [i] = dynLab. at (i);
	for (; i < COLS - 1; i ++)
	   text [i] = ' ';
	text [COLS - 1] = 0;
	writeMessage (LINES - 1, 0, text);
}

