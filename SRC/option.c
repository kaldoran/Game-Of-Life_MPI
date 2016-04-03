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
#include <getopt.h>

#include "game.h"
#include "option.h"

void usage(char* name) {
    printf("%s [-h]\n\t\t [-f <filePath>] [-t <maxTick>] [-c <number cols] [-r <number rows] [-g] [-n] [-s]\n\n", name);
    printf("\t\t -h : print this help\n");
    printf("\t\t -f filePath : path to the file to use for the grid\n");
    printf("\t\t -t maxTick : max time to make the game tick, set it to negatif for infinite tick\n");
    printf("\t\t -c : total numner of column\n");
    printf("\t\t -r : total number of rows\n");
    printf("\t\t -n : use ncurses for the display\n");
    printf("\t\t -g : if -g set then we use fine grained method\n");
    printf("\t\t -s : if -s is use the final grib will be saved\n");

    exit(EXIT_SUCCESS);
}

/**
 * Private function that define the default value for the option
 * %return : The option struct with the default value
 */
Option __setDefaultValue() {
    Option o;
    
    o.use_fine_grained = false;
    o.use_ncurses = false;
    o.file_path = "\0";
    o.max_tick = 100;
    o.nb_thread = 0;
    o.save_file = false;

    o.rows = rand() % ( MAX_ROWS_SIZE - MIN_ROWS_SIZE) + MIN_ROWS_SIZE;
    o.cols = rand() % ( MAX_COLS_SIZE - MIN_COLS_SIZE) + MIN_COLS_SIZE;    
    
    return o;
}

Option getOption(int argc, char **argv) {
    int opt = 0;
    Option o = __setDefaultValue();
    
    while ( (opt = getopt(argc, argv, OPT_LIST)) != -1 ) {
        switch(opt) {
            case '?':
            case 'h': 
                usage(argv[0]);
                break;
            case 'f':
                if ( optarg != 0 )
                    o.file_path = optarg;
                break;
            case 't':
                o.max_tick = atoi(optarg);
                break;
            case 'n': 
                o.use_ncurses = true;
                #ifndef PRINT
                    printf("You need to compile with make display to use '-n' parameter\n");
                #endif
                break;
            case 'p':
                o.nb_thread = MAX(atoi(optarg), 0);
                break;
            case 'r':
                o.rows = MAX(atoi(optarg), MIN_ROWS_SIZE);
                break;
            case 'c':
                o.cols = MAX(atoi(optarg), MIN_COLS_SIZE);
                break;
            case 'g':
                o.use_fine_grained = true;
                break;
            case 's': 
                o.save_file = true;
                break;
            default: 
                exit(EXIT_FAILURE);
        }
    }
    
    if ( argc == 1) 
        fprintf(stderr, "Remember to use -h for help\n");

    return o;
}

