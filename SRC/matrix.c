#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <mpi.h>

#include "memory.h"
#include "error.h"
#include "game.h"

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
