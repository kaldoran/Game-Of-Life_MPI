
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <mpi.h>

#include "memory.h"
#include "game_struct.h"


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
