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
/* FICHIER : rows.h                                          */
/*---------------------------------------------------------- */

#ifndef ROWS_H
#define ROWS_H

#include "game_struct.h"

/**
 * Function which return the pointer to the string s with the offset "offset"
 * %param s : Object on which you need to do the offset
 * %param offset : Size of the offset that need to be done
 * %param object_size : Size of an object in the object
 * %return : The offset pointer 
 */
char *__offset(char *s, int offset);

/**
 * I choose to use the notation for private function for this one, since ONLY main should use it
 *
 * Halt private Function which offset the input buffer, only if needed (i.e. if process == 0 )
 * Cause all buffer ( except the first one ) got a buffer which offset the value of the rows
 * %param my_id : id of the current process
 * %param s : String to offset if needed
 * %param offset : Amount of offset that need to be done
 * %return : The offset pointer of the string
 */
char *__posBufferRecv(int my_id, char* s, int offset);

/** 
 * Function which share the border to all other process and get border of other process
 * %param s : Game where you going to share your border and get some
 * %param slice_size : Size of the slice of each subprocess [all should be equal ]
 * %param total_proc : total number of processus
 */
void shareGetBorder(Game *s, int slice_size, int my_id, int total_proc);

#endif
