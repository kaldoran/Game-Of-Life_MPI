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
    
    if ( my_id != 0 ) {
        MPI_Send(__offset(s->board, s->rows, sizeof(s->board[0])),
                 s->rows, MPI_CHAR, my_id - 1, 0, MPI_COMM_WORLD); 
    }

    if ( my_id != total_proc - 1) { 
        bottom_row = NEW_ALLOC_K(s->rows, char);
        /* Received the bottom row of the top process */
        MPI_Recv(bottom_row, s->rows, MPI_CHAR, my_id + 1, 0, 
                 MPI_COMM_WORLD, MPI_STATUS_IGNORE); 
        
        /* Send the bottom row of our slice */
        MPI_Send(__offset(s->board, s->rows * (slice_size - (my_id == 0)), sizeof(s->board[0])), s->rows, MPI_CHAR, my_id + 1, 0, MPI_COMM_WORLD);
        
        /* Replace our bottom row by the top row of the previous process */
        memcpy(__offset(s->board, s->rows * (slice_size + (my_id != 0)) , sizeof(s->board[0])), bottom_row, s->rows);
    }
    
    if ( my_id != 0 ) {
        MPI_Recv(s->board, s->rows, MPI_CHAR, my_id - 1, 0,
                 MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }
    
}

/**
 * Private function which return a submatrix according to a process id
 * %param g : Game board to split
 * %param id : Id of the process ( which one we need a submatrix for)
 * %param slice_size : Size of the submatrix
 * %param proc_slice : Size of the slice of proc
 * %return : The submatrix associate with the id
 */
Game* __getSubMatrice(Game *g, int id, unsigned int slice_size, unsigned int proc_slice ) {
    Game *s = NULL;
    unsigned int x, y, startx, starty;
   
    startx = (id / proc_slice) * slice_size;
    starty = (id % proc_slice) * slice_size;
    s = newGame(slice_size, slice_size);
    
    DEBUG_MSG("[%d] Will start at : [%d, %d]\n", id, startx, starty);
    
    for (x = 0; x < slice_size; x++ ) 
        for (y = 0; y < slice_size; y++ ) 
            s->board[POS(x, y, s)] = g->board[POS(startx + x, starty + y, g)];
    
    return s;
}

/** 
 * Function which send matrix to other process
 * %param g : Original game board to split 
 * %return : A part of the matrix to work for process 0
 */
Game* sendAllSubMatrice(Game *g) {
    Game *tmp = NULL;
    int total_proc, i;
    unsigned proc_slice, slice_size;

    MPI_Comm_size(MPI_COMM_WORLD, &total_proc);    

    proc_slice = sqrt(total_proc);
    slice_size = g->rows / proc_slice;
    
    for ( i = 1; i < total_proc; i++) {
        tmp = __getSubMatrice(g, i, slice_size, proc_slice ); 
        MPI_Send(tmp->board, tmp->rows * tmp->cols, MPI_CHAR, i, 0, MPI_COMM_WORLD);
        free(tmp);
    }   
    
    return __getSubMatrice(g, 0, slice_size, proc_slice);

}

/**
 * Private function which replace the received matrix and make space for border
 * %param tmp : Tmp board which contains the received matrix
 * %param rows_size : Total number of rows of original matrix 
 * %param cols_size : Total number of column of original matrix
 * %param slice_size : Slice size of a submatrix
 * %return : Matrix received with buffer for sharing
 */
Game* __initPlaceBoard(Game *tmp, int rows_size, int cols_size, unsigned int slice_size) {
    Game *s = NULL;
    int total_proc, my_id;
    unsigned int proc_slice, startx, starty, xost, yosl, xosb, yosr, x, y;

    MPI_Comm_rank(MPI_COMM_WORLD, &my_id);
    MPI_Comm_size(MPI_COMM_WORLD, &total_proc);   

    proc_slice = sqrt(total_proc);

    startx = (my_id / proc_slice) * slice_size;
    starty = (my_id % proc_slice) * slice_size;
 
    xost = ( startx > 0 ); /* x offset on top */
    yosl = ( starty > 0 ); /* y offset on left */

    xosb = ( startx + slice_size != ((unsigned) cols_size) ); /* bottom */
    yosr = ( starty + slice_size != ((unsigned) rows_size) ); /* right  */
    
    s = newGame(slice_size + yosr + yosl
              , slice_size + xost + xosb );
    
    for (x = 0; x < slice_size; x++ ) {
        for (y = 0; y < slice_size; y++ ) {
            s->board[POS(x + xost, y + yosl, s)] = tmp->board[POS(x, y, tmp)];
        }
    }

    return s;
}

/**
 * Received a matrix send by process 0
 * %param rows_size : Total number of rows of original matrix 
 * %param cols_size : Total number of column of original matrix
 * %return : return the board including the offset for border sharing
 */
Game* receivedMatrix(int rows_size, int cols_size) {
    Game *s, *tmp;
    int total_proc; 
    unsigned int slice_size;

    MPI_Comm_size(MPI_COMM_WORLD, &total_proc);   
    slice_size = rows_size / sqrt(total_proc); 
    tmp = newGame(slice_size, slice_size );

    MPI_Recv(tmp->board, slice_size * slice_size, MPI_CHAR, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    s = __initPlaceBoard(tmp, rows_size, cols_size, slice_size);

    freeGame(tmp);
   
    return s;
}


int main(int argc, char* argv[]) {

    Option o;
    Game *g, *s = NULL;
    double time_taken = 0.0;
    int total_proc, my_id, slice_size, size_tick[4];

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD,&total_proc);
    MPI_Comm_rank(MPI_COMM_WORLD,&my_id);
 
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
        if ( my_id == 0 && ( size_tick[0] % total_proc != 0 
            || size_tick[1] != size_tick[0] ) ) {

            QUIT_MSG("Grid could no be devided by the total number of process\n");
        }

        if ( my_id == 0 ) 
            s = sendAllSubMatrice(g);
        else 
            s = receivedMatrix(size_tick[0], size_tick[1]);
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
    
    for ( ; size_tick[2] >= 0; size_tick[2]--) {
       
        /* This pre-process indication is defined by the make display command */

        /* If we need to display, Then we going to print */
        #if PRINT
        if ( my_id == 0 )
            gamePrintInfo(g, 0);
        #endif

        /* Time to share border */
        if ( size_tick[3] == DIVIDE_MATRICE )
            ; /* Share border too */
        else
            shareGetBorder(s, slice_size, my_id, total_proc);

        processGameTick(s);

        /* If we need to print, then we will send all data to process 0 */
        #if PRINT
        /* Time to send all data to process 0 */
        if ( size_tick[3] == DIVIDE_MATRICE ) 
            ; /* All send matrix to 0 */ 
        else
            MPI_Gather( __posBufferRecv(my_id, s->board, size_tick[0]),
                        size_tick[0] * slice_size, MPI_CHAR,
                        g->board, size_tick[0] * slice_size, MPI_CHAR,
                        0, MPI_COMM_WORLD);
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

