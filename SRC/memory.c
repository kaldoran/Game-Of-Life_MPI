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

#include "error.h"
#include "memory.h"

/**
 * Private function that board the allocation of an object
 * %param total : Total number of object that we need to allocate 
 * %param object_size : Size of the object which we need to allocate
 * %return : Pointer on the memory associate with the new object
 */
void *__memAlloc(int total, size_t object_size) {

    void *p = calloc(total, object_size);

    if ( p == NULL ) 
        QUIT_MSG("Canno't allocate new object\n");

    return p;

}
