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
 
#include <stdlib.h>
#include <mpi.h>

#include "rows.h"
#include "game_struct.h"

char *__offset(char *s, int offset) {
    return &(*(s + (offset * sizeof(char)) ));
}

char *__posBufferRecv(int my_id, char* s, int offset) {
    if ( my_id != 0 ) 
        return __offset(s, offset);
    return s;
}


void shareGetBorder(Game *s, int slice_size, int my_id, int total_proc) {
    
    if ( my_id != 0 ) { /* send bottom row to process on top */
        MPI_Send(__offset(s->board, s->rows),
                 s->rows, MPI_CHAR, my_id - 1, 0, MPI_COMM_WORLD); 
    }

    if ( my_id != total_proc - 1) { 
        /* Received the top row of the bottom process */
        MPI_Recv(__offset(s->board, s->rows * (slice_size + (my_id != 0))),
                 s->rows, MPI_CHAR, my_id + 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);   
        
        /* Send the bottom row of our slice */
        MPI_Send(__offset(s->board, s->rows * (slice_size - (my_id == 0))), 
                 s->rows, MPI_CHAR, my_id + 1, 0, MPI_COMM_WORLD);
    }
    
    if ( my_id != 0 ) { /* Recv the bottom row of the process at top*/
        MPI_Recv(s->board, s->rows, MPI_CHAR, my_id - 1, 0,
                 MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }
   
    MPI_Barrier(MPI_COMM_WORLD);
}
