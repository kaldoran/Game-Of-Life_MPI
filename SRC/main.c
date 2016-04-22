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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curses.h>
#include <unistd.h>
#include <math.h>
#include <time.h>
#include <mpi.h>

#include "memory.h"
#include "error.h"
#include "game.h"
#include "option.h"

char *__offset(char *s, int offset, size_t object_size) {
    return &(*(s + (offset * object_size) ));
}

char *__posBufferRecv(int my_id, char* s, int offset) {
    if ( my_id != 0 ) 
        return __offset(s, offset, sizeof(s[0]));
    return s;
}


void shareGetBorder(Game *s, int slice_size, int my_id, int total_proc) {
    char *bottom_row = "";
    
    if ( my_id != 0 ) { /* send bottom row to process on top */
        MPI_Send(__offset(s->board, s->rows, sizeof(s->board[0])),
                 s->rows, MPI_CHAR, my_id - 1, 0, MPI_COMM_WORLD); 
    }

    if ( my_id != total_proc - 1) { 
        bottom_row = NEW_ALLOC_K(s->rows, char);
        /* Received the top row of the bottom process */
        MPI_Recv(bottom_row, s->rows, MPI_CHAR, my_id + 1, 0, 
                 MPI_COMM_WORLD, MPI_STATUS_IGNORE);   
        
        /* Send the bottom row of our slice */
        MPI_Send(__offset(s->board, s->rows * (slice_size - (my_id == 0)), 
                 sizeof(s->board[0])), s->rows, MPI_CHAR, my_id + 1, 0, MPI_COMM_WORLD);
        
        /* Replace our bottom row by the top row of the previous process */
        memcpy(__offset(s->board, s->rows * (slice_size + (my_id != 0)) , sizeof(s->board[0])), bottom_row, s->rows);
    }
    
    if ( my_id != 0 ) { /* Recv the bottom row of the process at top*/
        MPI_Recv(s->board, s->rows, MPI_CHAR, my_id - 1, 0,
                 MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }
   
    MPI_Barrier(MPI_COMM_WORLD);
}

/**
 * Private function which return a sub-matrix according to a start on x and y and slice size
 * %param src : Source matrix which we extract data
 * %param startx : Where to start on x
 * %param starty : Where to start on y
 * %param xslice_size : Slice size on x
 * %param yslice_size : Slice size on y
 * %return : The submatrix of size xslice_size * yslice_size starting at startx, starty
 */
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

    for (x = 0; x < src->cols; x++ ) {
        for (y = 0; y < src->rows; y++ ) {
            dest->board[POS(x + startx, y + starty, dest)] = src->board[POS(x, y, src)];
        }
    }
}

/*  Matrix : 3 x 3, Np = 9 
 *  [V][S] [S][V][S] [S][V] < x: 0
 *  [S][S] [S][S][S] [S][S]  
 *
 *  [S][S] [S][S][S] [S][S] < x: 1
 *  [V][S] [S][V][S] [S][V] < 
 *  [S][S] [S][S][S] [S][S] < 
 *
 *  [S][S] [S][S][S] [S][S] < x: 2
 *  [V][S] [S][V][S] [S][V] < 
 *   y: 0    y: 1     y: 2
 *  
 *  S = Buffer   
 *  V = Value of original 3 x 3 matrix  
 */


void shareMatrixBorder(Game *s, int my_x, int my_y, int slice_size, int proc_slice ) {
    int my_id;
    Game *tmp, *buf;
    tmp = NULL;
    buf = newGame(1, slice_size);
    
    my_id = my_y + my_x * proc_slice;
    if ( my_y != proc_slice - 1) { /* Send right column */
        tmp = __subMatrix(s, (my_x != 0), slice_size - (my_y == 0), slice_size, 1);
        MPI_Send(tmp->board, tmp->rows * tmp->cols, MPI_CHAR, my_id + 1, 0, MPI_COMM_WORLD);
        free(tmp);
    }

    if ( my_y != 0 ) { /* get right column and send our left */
        MPI_Recv(buf->board, buf->cols * buf->rows, MPI_CHAR, my_id - 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        tmp = __subMatrix(s, (my_x != 0), 1, slice_size, 1); /* Start at 1 due to buffer */
        
        MPI_Send(tmp->board, tmp->rows * tmp->cols, MPI_CHAR, my_id - 1, 0, MPI_COMM_WORLD);   
        __mergeMatrix(buf, s, (my_x != 0), 0);
        free(tmp);
    }


    if ( my_y != proc_slice - 1) { /* get the left column of neighbours */
        MPI_Recv(buf->board, buf->cols * buf->rows, MPI_CHAR, my_id + 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        __mergeMatrix(buf, s, (my_x != 0), slice_size + 1 - (my_y == 0));
    }

    freeGame(buf);
    buf = newGame(slice_size, 1);
   
    /* TODO */
    if ( my_x != proc_slice - 1) {
        tmp = __subMatrix(s, slice_size - (my_x == 0), (my_y != 0), 1, slice_size);
        MPI_Send(tmp->board, tmp->rows * tmp->cols, MPI_CHAR, my_id + proc_slice, 0, MPI_COMM_WORLD);
        free(tmp);
    }

    if ( my_x != 0 ) {
        MPI_Recv(buf->board, buf->cols * buf->rows, MPI_CHAR, my_id - proc_slice, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        tmp = __subMatrix(s, 1, (my_y != 0), 1, slice_size); /* Start at 1 due to buffer */
        
        MPI_Send(tmp->board, tmp->rows * tmp->cols, MPI_CHAR, my_id - proc_slice, 0, MPI_COMM_WORLD);   
        __mergeMatrix(buf, s, 0, (my_y != 0));
        free(tmp);
    }

    if ( my_x != proc_slice - 1 ) {
        MPI_Recv(buf->board, buf->cols * buf->rows, MPI_CHAR, my_id + proc_slice, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        __mergeMatrix(buf, s, slice_size + 1 - (my_x == 0), (my_y != 0));
    }
    gamePrintInfo(s, my_id);
    free(buf);
}

void gatherMatrix(Game *g, Game *s, int my_x, int my_y, int slice_size, int proc_slice, int total_proc) {
    Game* tmp = NULL;
    
    tmp = __subMatrix(s, (my_x != 0), (my_y != 0), slice_size, slice_size);
    MPI_Send(tmp->board, tmp->rows * tmp->cols, MPI_CHAR, 0, 0, MPI_COMM_WORLD);
    free(tmp);

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
        
        free(tmp);
    }
    /* Nobody will go out since process 0 end recv */
    MPI_Barrier(MPI_COMM_WORLD);
}


/** 
 * Function which send matrix to other process
 * %param g : Original game board to split 
 * %return : A part of the matrix to work for process 0
 */
void sendAllSubMatrice(Game *g, int slice_size, int proc_slice) {
    Game *tmp = NULL;
    int total_proc, i, is_x, is_y;

    MPI_Comm_size(MPI_COMM_WORLD, &total_proc);    

    for ( i = 0; i < total_proc; i++) {
        is_x = i / proc_slice; 
        is_y = i % proc_slice;

        tmp = __subMatrix(g, is_x * slice_size, is_y * slice_size, slice_size, slice_size ); 
        MPI_Send(tmp->board, tmp->rows * tmp->cols, MPI_CHAR, i, 0, MPI_COMM_WORLD);
        free(tmp);
    }    
}

/**
 * Received a matrix send by process 0
 * %param rows_size : Total number of rows of original matrix 
 * %param cols_size : Total number of column of original matrix
 * %return : return the board including the offset for border sharing
 */
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


int main(int argc, char* argv[]) {

    Option o;
    Game *g, *s = NULL;
    double time_taken = 0.0;
    int total_proc, my_id, my_x, my_y, proc_slice, slice_size, size_tick[4];

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD,&total_proc);
    MPI_Comm_rank(MPI_COMM_WORLD,&my_id);

    /* The process 0 get all parameters, load board if needed etc */
    if ( my_id == 0 ) {
        srand(time(NULL));
        o = getOption(argc, argv);  /* Get all option     */

        if ( *o.file_path != '\0' ) /* If path file is not empty */
            if ( (g = loadBoard(o.file_path)) == NULL ) /* then use the given file [load id] */
                fprintf(stderr, "Can't load file %s\n", o.file_path);

        if ( g == NULL ) /* If load of file fail Or no grid given */
            g = generateRandomBoard(o); /* then create one */

        time_taken = MPI_Wtime();

        /* Fill value that will be needed for other process */
        size_tick[0] = g->rows;
        size_tick[1] = g->cols;
        size_tick[2] = o.max_tick;
        size_tick[3] = o.method;

    }

    /* Broadcast all the needed value ( in a array for compacting data) */
    MPI_Bcast(size_tick, 4, MPI_INT, 0, MPI_COMM_WORLD);
   
    /********************/
    /*     init part    */
    /********************/

    if ( size_tick[3] == DIVIDE_MATRICE) {
        if ( my_id == 0 && ( ((size_tick[0] * size_tick[1]) % total_proc) != 0
            || size_tick[1] != size_tick[0] ) ) {

            QUIT_MSG("Grid could no be devided by the total number of process %d - %d\n", size_tick[1], total_proc);
        }

        proc_slice = sqrt(total_proc);
        slice_size = size_tick[0] / proc_slice;
        my_x = my_id / proc_slice;
        my_y = my_id % proc_slice;
         
        if ( my_id == 0 ) 
            sendAllSubMatrice(g, slice_size ,proc_slice);
         
        s = receivedMatrix(my_x, my_y, slice_size, proc_slice);
    } else {
        if ( my_id == 0 && size_tick[1] % total_proc != 0 )
            QUIT_MSG("Grid could no be devided by the total number of process\n");

        /* Lets allocated the memory for the shared buffer at the same moment */
        slice_size = size_tick[1] / total_proc;

        s = newGame(size_tick[0], 
                slice_size + ((my_id == 0 || my_id == total_proc - 1) ? 1 : 2) );

        MPI_Scatter( g->board, size_tick[0] * slice_size, MPI_CHAR,
                __posBufferRecv(my_id, s->board, size_tick[0]),
                size_tick[0] * slice_size, MPI_CHAR, 
                0, MPI_COMM_WORLD);
    }

    /********************/
    /*     Init end     */
    /* -----------------*/
    /*   Process tick   */
    /********************/
    
    /* This pre-process indication is defined by the make display command */
    #if PRINT
    if ( my_id == 0 )
        gamePrintInfo(g, size_tick[2]);
    #endif
    
    for ( ; size_tick[2] > 0; size_tick[2]--) {
       
        if ( size_tick[3] == DIVIDE_MATRICE ) {
            shareMatrixBorder(s, my_x, my_y, slice_size, proc_slice);
            processMatrixGameTick(s, my_x, my_y, slice_size);
            gatherMatrix(g, s, my_x, my_y, slice_size, proc_slice, total_proc);
        } else {
            shareGetBorder(s, slice_size, my_id, total_proc);
            processRowsGameTick(s);
            MPI_Gather( __posBufferRecv(my_id, s->board, size_tick[0]),
                        size_tick[0] * slice_size, MPI_CHAR,
                        g->board, size_tick[0] * slice_size, MPI_CHAR,
                        0, MPI_COMM_WORLD);
        }

        /* If we need to display, Then we going to print */
        #if PRINT
        if ( my_id == 0 )
            gamePrintInfo(g, size_tick[2]);
        #endif
    }
    
    if ( my_id == 0 ) {
        time_taken =  MPI_Wtime() - time_taken;
        printf("Time : %f\n", time_taken);

        if ( o.save_file )     
            saveBoard(g);

        freeGame(g);           /* Free space we are not in Java */
    }
    
    free(s);
    MPI_Finalize();

    exit(EXIT_SUCCESS);
}

