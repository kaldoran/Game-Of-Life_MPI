/*
 *  Copyright (C) 2016-2016 REYNAUD Nicolas
 *  Author : REYNAUD Nicolas <kaldoran [at] live.fr>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301 USA.
 */

/*---------------------------------------------------------- */
/* AUTEUR : REYNAUD Nicolas                                  */
/* FICHIER : error.h                                         */
/*---------------------------------------------------------- */


#ifndef OPT
#define OPT

#include "option_struct.h"

/* List of possible option */
#define OPT_LIST "hf:t:np:r:c:gs"

/** Use the definition defined by David Titarenco
 *  On StackOverFlow http://stackoverflow.com/questions/3437404/min-and-max-in-c 
 */
#define MAX(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a > _b ? _a : _b; })

/**
 * Print the usage of the program
 * %param name : name of the program
 */
void usage(char* name);

/**
 * Function that get all command line option and return those one into a structure
 * %param argc : Total number of argument onto the command line
 * %param argv : Contenant of all the command line
 * %return     : Structure which contains all option given onto command line into this structure
 */
Option getOption(int argc, char** argv);

#endif
