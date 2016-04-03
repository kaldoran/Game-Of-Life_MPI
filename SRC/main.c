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
#include <mpi.h>

#include "error.h"
#include "game.h"
#include "task.h"
#include "ncurses.h"
#include "option.h"
#include "thread.h"

int main(int argc, char* argv[]) {

    int totalThread, myId;
    
    Option o;
    double time;
    Game* g = NULL; 

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD,&totalThread);
    MPI_Comm_rank(MPI_COMM_WORLD,&myId);
  
    /* Then, here argc and argv are set to the right value */
    o = getOption(argc, argv);  /* Get all option     */

    if ( *o.file_path != '\0' ) /* If path file is not empty */
        if ( (g = loadBoard(o.file_path)) == NULL ) /* then use the given file [load id] */
            fprintf(stderr, "Can't load file %s\n", o.file_path);

    if ( g == NULL ) /* If load of file fail Or no grid given */
        g = generateRandomBoard(o); /* then create one */

    if ( o.use_ncurses ) /* If we use ncurses */
        initNCurses();   /* Then we init the display */

    time = MPI_Wtime();
    while(o.max_tick != 0) {         /* Inifinit loop if total tick not given */

        gamePrintInfo(g, o);         
        
        swapGrid(g);
        --o.max_tick;
        
        #ifdef PRINT                 /* If we print we add some delay without it we can't see the grid */
            usleep(400000);
        #endif
    }
    
    time =  MPI_Wtime() - time;
    printf("Time : %f\n", time);
    
    if ( o.use_ncurses) /* If we use ncurses ( and then init it ) */
        endNCurses();   /* we need to clear display info */

    if ( o.save_file )     
        saveBoard(g);

    MPI_Finalize();
    freeGame(g);           /* Free space we are not in Java */
    
    exit(EXIT_SUCCESS);
}

