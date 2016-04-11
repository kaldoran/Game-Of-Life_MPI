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
#include <time.h>
#include <assert.h>
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

int main(int argc, char* argv[]) {

    Option o;
    Game *g, *s = NULL;
    double time_taken = 0.0;
    int total_proc, my_id, slice_size, size_tick[3];

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD,&total_proc);
    MPI_Comm_rank(MPI_COMM_WORLD,&my_id);
 
    if ( my_id == 0 ) {
        g = NULL;
        srand(time(NULL));
        o = getOption(argc, argv);  /* Get all option     */

        if ( *o.file_path != '\0' ) /* If path file is not empty */
            if ( (g = loadBoard(o.file_path)) == NULL ) /* then use the given file [load id] */
                fprintf(stderr, "Can't load file %s\n", o.file_path);

        if ( g == NULL ) /* If load of file fail Or no grid given */
            g = generateRandomBoard(o); /* then create one */

        time_taken = MPI_Wtime();
        size_tick[0] = g->rows;
        size_tick[1] = g->cols;
        size_tick[2] = o.max_tick;

    }

    MPI_Bcast(size_tick, 3, MPI_INT, 0, MPI_COMM_WORLD);

    if ( o.method == DIVIDE_ROWS ) {
        
        assert( size_tick[1] % total_proc == 0 && "Erreur : La grille n'est pas divisible par le nombre de processus");
        
        for ( ; size_tick[2] >= 0; size_tick[2]--) {
            
            if ( my_id == 0 )
                gamePrintInfo(g, 0);
            
            /* Lets allocated the memory for the shared buffer at the same moment */
            slice_size = size_tick[1] / total_proc;
            s = newGame(size_tick[0], 
                    slice_size + (( my_id == 0 || my_id == total_proc - 1) ? 1 : 2) );

            MPI_Scatter( g->board, size_tick[0] * slice_size, MPI_CHAR,
                    __posBufferRecv(my_id, s->board, size_tick[0]),
                    size_tick[0] * slice_size, MPI_CHAR, 
                    0, MPI_COMM_WORLD);
            
            shareGetBorder(s, slice_size, my_id, total_proc);

            processGameTick(s);

            MPI_Gather( __posBufferRecv(my_id, s->board, size_tick[0]),
                    size_tick[0] * slice_size, MPI_CHAR,
                    g->board, size_tick[0] * slice_size, MPI_CHAR,
                    0, MPI_COMM_WORLD);

        }
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

