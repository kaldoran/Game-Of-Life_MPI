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
#include <math.h>
#include <time.h>
#include <mpi.h>

#include "matrix.h"
#include "rows.h"
#include "memory.h"
#include "error.h"
#include "game.h"
#include "option.h"


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
        proc_slice = sqrt(total_proc);
        slice_size = size_tick[0] / proc_slice;
        my_x = my_id / proc_slice;
        my_y = my_id % proc_slice;
        
        if ( my_id == 0 && ( fmod(sqrt(total_proc), 1.0f) != 0
                    || size_tick[1] != size_tick[0] ) ) {

            QUIT_MSG("Grid could no be devided by the total number of process %d - %d\n", size_tick[1], total_proc);
        }
 
        if ( my_id == 0 ) 
            sendAllSubMatrice(g, slice_size, proc_slice);
         
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

