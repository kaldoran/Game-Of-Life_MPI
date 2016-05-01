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
#include <stdio.h>
#include <math.h>
#include <mpi.h>

#include "matrix.h"
#include "memory.h"
#include "error.h"
#include "game.h"

Game *__subMatrix(Game *src, int startx, int starty, int xslice_size, int yslice_size) {
    Game *dest = NULL;
    int x, y;

    dest = newGame(yslice_size, xslice_size);
    for ( x = 0; x < xslice_size; x++ )
        for ( y = 0; y < yslice_size; y++ )
            dest->board[POS(x, y, dest)] = src->board[POS(x + startx, y + starty, src)];

    return dest;
}

void __mergeMatrix(Game *src, Game *dest, int startx, int starty) {
    unsigned int x, y;

    for (x = 0; x < src->cols; x++ )
        for (y = 0; y < src->rows; y++ )
            dest->board[POS(x + startx, y + starty, dest)] = src->board[POS(x, y, src)];
}

void shareMatrixBorder(Game *s, int my_x, int my_y, int slice_size, int proc_slice ) {
    int my_id;
    Game *tmp, *buf;
    tmp = NULL;
    buf = newGame(1, slice_size);
    
    my_id = my_y + my_x * proc_slice;
    
    /* Send Right - Left*/
    if ( my_y != proc_slice - 1) { /* Send right column */
        tmp = __subMatrix(s, (my_x != 0), slice_size - (my_y == 0), slice_size, 1);
        MPI_Send(tmp->board, tmp->rows * tmp->cols, MPI_CHAR, my_id + 1, 0, MPI_COMM_WORLD);
        freeGame(tmp);
    }

    if ( my_y != 0 ) { /* get right column and send our left */
        MPI_Recv(buf->board, buf->cols * buf->rows, MPI_CHAR, my_id - 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        tmp = __subMatrix(s, (my_x != 0), 1, slice_size, 1); /* Start at 1 due to buffer */
        
        MPI_Send(tmp->board, tmp->rows * tmp->cols, MPI_CHAR, my_id - 1, 0, MPI_COMM_WORLD); 
        __mergeMatrix(buf, s, (my_x != 0), 0);
        freeGame(tmp);
    }

    if ( my_y != proc_slice - 1) { /* get the left column of neighbours */
        MPI_Recv(buf->board, buf->cols * buf->rows, MPI_CHAR, my_id + 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        __mergeMatrix(buf, s, (my_x != 0), slice_size + 1 - (my_y == 0));
    }

    freeGame(buf);
    buf = newGame(slice_size, 1);
   
    /* Send Top-Bottom */
    if ( my_x != proc_slice - 1) {
        tmp = __subMatrix(s, slice_size - (my_x == 0), (my_y != 0), 1, slice_size);
        MPI_Send(tmp->board, tmp->rows * tmp->cols, MPI_CHAR, my_id + proc_slice, 0, MPI_COMM_WORLD);
        freeGame(tmp);
    }

    if ( my_x != 0 ) {
        MPI_Recv(buf->board, buf->cols * buf->rows, MPI_CHAR, my_id - proc_slice, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        tmp = __subMatrix(s, 1, (my_y != 0), 1, slice_size); /* Start at 1 due to buffer */
        
        MPI_Send(tmp->board, tmp->rows * tmp->cols, MPI_CHAR, my_id - proc_slice, 0, MPI_COMM_WORLD); 
        __mergeMatrix(buf, s, 0, (my_y != 0));
        freeGame(tmp);
    }
    if ( my_x != proc_slice - 1 ) {
        MPI_Recv(buf->board, buf->cols * buf->rows, MPI_CHAR, my_id + proc_slice, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        __mergeMatrix(buf, s, slice_size + 1 - (my_x == 0), (my_y != 0));
    }
    
    freeGame(buf);

    /* Diagonales */
    if ( my_y != 0 && my_x != 0 ) /* Send to top left */
        MPI_Send(&s->board[POS(1, (my_y != 0), s)], 1, MPI_CHAR, my_id - proc_slice - 1, 0, MPI_COMM_WORLD);
    

    if ( my_y != proc_slice - 1 && my_x != proc_slice - 1) { /* Send to bottom right */
        /* Get the top left element */
        MPI_Recv(&s->board[POS(slice_size + (my_x != 0), slice_size, s)], 1, MPI_CHAR, 
                 my_id + proc_slice + 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        MPI_Send(&s->board[POS(slice_size - (my_x == 0), slice_size - (my_y == 0), s)], 1, MPI_CHAR, 
                 my_id + proc_slice + 1, 0, MPI_COMM_WORLD);

    }

    if ( my_y != 0 && my_x != 0 ) {
        MPI_Recv(&s->board[POS(0, 0, s)], 1, MPI_CHAR, 
                 my_id - proc_slice - 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }

    if ( my_y != proc_slice - 1 && my_x != 0 ) /* Send to top right */
        MPI_Send(&s->board[POS(1, slice_size - (my_y == 0), s)], 1, MPI_CHAR, my_id - proc_slice + 1, 0, MPI_COMM_WORLD); 

    if ( my_y != 0 && my_x != proc_slice - 1 ) { /* Send  to bottom left */
        MPI_Recv(&s->board[POS(slice_size + (my_x != 0) ,0, s)], 1, MPI_CHAR, 
                 my_id + proc_slice - 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        MPI_Send(&s->board[POS(proc_slice - (my_x == 0),1, s)], 1, MPI_CHAR, my_id + proc_slice - 1, 0, MPI_COMM_WORLD);
    }

    if ( my_y != proc_slice - 1 && my_x != 0 ) {
        MPI_Recv(&s->board[POS(0, slice_size + (my_y != 0), s)], 1, MPI_CHAR, 
                 my_id - proc_slice + 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }

}

void gatherMatrix(Game *g, Game *s, int my_x, int my_y, int slice_size, int proc_slice, int total_proc) {
    Game* tmp = NULL;
    
    tmp = __subMatrix(s, (my_x != 0), (my_y != 0), slice_size, slice_size);
    MPI_Send(tmp->board, tmp->rows * tmp->cols, MPI_CHAR, 0, 0, MPI_COMM_WORLD);
    freeGame(tmp);

    if ( my_x == 0 && my_y == 0 ) {
        int total_recv;
        MPI_Status status;

        tmp = newGame(slice_size, slice_size); 
        for ( total_recv = 0; total_recv < total_proc; total_recv++ ) {
            MPI_Recv(tmp->board, tmp->rows * tmp->cols, MPI_CHAR, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &status);
            __mergeMatrix(tmp, g, 
                    (status.MPI_SOURCE / proc_slice) * slice_size, 
                    (status.MPI_SOURCE % proc_slice) * slice_size);
        }
        
        freeGame(tmp);
    }
    /* Nobody will go out since process 0 end recv */
    MPI_Barrier(MPI_COMM_WORLD);
}
 
void sendAllSubMatrice(Game *g, int slice_size, int proc_slice) {
    Game *tmp = NULL;
    int total_proc, i, is_x, is_y;

    MPI_Comm_size(MPI_COMM_WORLD, &total_proc);    

    for ( i = 0; i < total_proc; i++) {
        is_x = i / proc_slice; 
        is_y = i % proc_slice;

        tmp = __subMatrix(g, is_x * slice_size, is_y * slice_size, slice_size, slice_size ); 
        MPI_Send(tmp->board, tmp->rows * tmp->cols, MPI_CHAR, i, 0, MPI_COMM_WORLD);
        freeGame(tmp);
    }    
}

Game* receivedMatrix(int my_x, int my_y, int slice_size, int proc_slice) {
    Game *s, *tmp;

    tmp = newGame(slice_size, slice_size );
    s = newGame(slice_size + (my_y != 0) + (my_y != proc_slice - 1),
                slice_size + (my_x != 0) + (my_x != proc_slice - 1));

    MPI_Recv(tmp->board, slice_size * slice_size, MPI_CHAR, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    __mergeMatrix(tmp, s, (my_x != 0), (my_y != 0)); 
    freeGame(tmp);
  
    return s;
}

