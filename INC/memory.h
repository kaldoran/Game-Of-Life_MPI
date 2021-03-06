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
/* FICHIER : memory.h                                        */
/*---------------------------------------------------------- */


#ifndef MEMORY_H
#define MEMORY_H

#include <stdlib.h>

/**
 * Function that allocate a single object
 * %param OBJECT : Object type to allocate
 * %return       : Pointer in memory associate with the object Type.
 */
#define NEW_ALLOC(OBJECT) (NEW_ALLOC_K(1, OBJECT))

/**
 * Function that allocate an array of the same Object
 * %param K      : Total number to allocate
 * %param OBJECT : Object type to allocate
 * %return       : Pointer in memory associate with the object type.
 */
#define NEW_ALLOC_K(K, OBJECT) (__memAlloc(K, sizeof(OBJECT)))

/**
 * Private function that shouldn't be used 
 * The definition of this function is in memory.c
 */
void *__memAlloc(int total, size_t object_size);

#endif
