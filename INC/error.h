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

#ifndef ERROR_H
#define ERROR_H

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

/** 
 * If Debug Flag is on, create a maccro to print debug information
 * %param MSG : String to print 
 * %param ... : List of param [ for example if want to print variable value ]
 */
#ifdef DEBUG
    #define DEBUG_MSG(MSG, ...)   \
    do {                          \
        fprintf(stderr, "\n\t[DEBUG] File : %s - Line : %d - Function : %s() : " MSG "\n", __FILE__, __LINE__, __func__, ## __VA_ARGS__);   \
    } while(0);
#else
    #define DEBUG_MSG(MSG, ...)
#endif

/** 
 * Create a maccro for quit the program 
 * %param MSG : String to print 
 * %param ... : List of param [ for example if want to print variable value ]
 */
#define QUIT_MSG(MSG, ...)                        \
    do {                                          \
        DEBUG_MSG(MSG, ##__VA_ARGS__)             \
        fprintf(stderr, "[FATAL ERROR] ");        \
        fprintf(stderr, MSG, ## __VA_ARGS__);     \
        perror(NULL);                             \
        exit(EXIT_FAILURE);                       \
    }while(0); 

#endif /* ERROR_H included */

