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
#ifndef	__DISPLAY_HANDLER__
#define	__DISPLAY_HANDLER__

#include	<string>

class	displayHandler {
public:
		displayHandler		();
		~displayHandler		();
void		startDisplay		();
void		stop			();
void		showTime		(const std::string &);
void		showHeader		(const std::string &);
void		showEnsemble		(const std::string &,
	                                 const std::string &);
void		showChannel		(const std::string &);
void		showService		(const std::string &, int);
void		clearServices		(int);
void		show_playing		(const std::string &);
std::string	getProtectionLevel	(bool shortForm, int16_t protLevel);
std::string	getCodeRate		(bool shortForm, int16_t protLevel);
void		show_audioData		(audiodata *ad);
void		clear_audioData 	();
void		mark_service		(int index, const std::string &);
void		show_dynamicLabel	(const std::string &dynLab);
private:
void		writeMessage		(int row, int column,
	                                            const char *message);
};
#endif
