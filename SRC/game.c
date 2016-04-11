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
#include <curses.h>
#include <string.h>
#include <stdbool.h>
#include <mpi.h>

#include "error.h"
#include "game.h"
#include "game_struct.h"
#include "memory.h"

/**
 * Private function that compute the position of the board given a x and a y
 * %param x : Position on the X coordinate 
 * %param y : Position on the Y coordinate
 * %param g : Game where we need to compute the cell position
 * %return  : Position of the cell associate with the X and Y coordinate
 */
int __position(unsigned int x, unsigned int y, Game* g) {
    return g->rows * x + y;
}

/**
 * Private function that print a simple line
 * %param g : Game structure which contains the information relative to the game
 */
void __printLine(Game* g) {
    unsigned int i = 0;

    printf( "+");
    for ( i = 0; i < g->cols + 2; i++ ) /* the 2 '+'  */
        printf( "-");
    printf( "+\n");

}

/**
 * Private function that really print the board contenant 
 * %param g : Game struct which contains the board to print
 * %param pf : Pointer to a printing function
 */
void __gamePrint (Game* g) {
    unsigned int x, y;

    printf( "Board size : \n");
    printf( "  %d Columns\n", g->cols);
    printf( "  %d rows\n", g->rows);
    __printLine(g);

    for ( x = 0; x < g->cols; x++) {
        printf( "| ");
        for ( y = 0; y < g->rows; y++) {
            printf( "%c", ((g->board[POS(x, y, g)] == DEAD_CELL) ? '.' : '#'));    
        }
        
        printf( " |\n");
    }
    
    __printLine(g);
    
    DEBUG_MSG("Print board finish\n");
}

void gamePrintInfo(Game* g, int tick_left) {
    
    DEBUG_MSG("Board : %s\n", g->board);

    #ifndef PRINT
         return;
    #endif

    if ( tick_left >= 0 )
        printf("%d Generation left.\n", tick_left);
    
    __gamePrint(g);
}

/** 
 * Private function that allocate a new board
 * %param rows : Total number of rows onto the board
 * %param cols : Total number of column onto the board
 * %return     : Allocated array of char which will contains the board
 */
char* __newBoard(unsigned int rows, unsigned int cols) {
    char* board = NEW_ALLOC_K(rows * cols, char);
    memset(board, DEAD_CELL, rows * cols);
    return board;
}

Game* newGame(unsigned int rows, unsigned int cols) {
    Game* g = NEW_ALLOC(Game);
    
    g->rows = rows;
    g->cols = cols;
    
    g->board = __newBoard(rows, cols);
    return g;
}

void freeGame(Game* g)  {
    if ( g == NULL ) 
        return;

    free(g->board);
    free(g);
}

Game* generateRandomBoard(Option o) {

    unsigned int rows = 0, cols = 0;
    Game* g;
        
    g = newGame(o.rows, o.cols);
    
    DEBUG_MSG("Ligne : %d, Cols : %d\n", o.rows, o.cols);
    for(cols = 0; cols < g->cols; cols++)
        for (rows = 0; rows < g->rows; rows++)
            g->board[POS(cols, rows, g)] = (
                ( rand() % 100 >= POURCENT_BEEN_ALIVE ) ? 
                    DEAD_CELL: 
                    ALIVE_CELL
                );            
    DEBUG_MSG("Generate random finish");
    return g;
}

/**
 * Private function which compute the total number of neighbour of a cell
 * %param x : X position of the cell on the board
 * %param y : Y position of the cell on the board
 * %param g : Game struct wich contains all information relative to the game
 * %return  : Total number of neighbour of this cell
 */
int __neighbourCell(unsigned int x, unsigned int y, Game *g) {
    unsigned int total = 0;
    char *b = g->board;
    bool isTop, isBot;

    isTop = (x == 0);
    isBot = (x == g->cols - 1);

    if ( y % g->rows != g->rows - 1) {
        total +=               (b[POS(x, y + 1, g)]     == ALIVE_CELL); /* Right */
        if ( !isBot ) total += (b[POS(x + 1, y + 1, g)] == ALIVE_CELL); /* Right - Down */
        if ( !isTop ) total += (b[POS(x - 1, y + 1, g)] == ALIVE_CELL); /* Up - Right */
    }
    
    if ( y % g->rows != 0 ) { 
        total +=               (b[POS(x, y - 1, g)]     == ALIVE_CELL); /* Left */
        if ( !isBot ) total += (b[POS(x + 1, y - 1, g)] == ALIVE_CELL); /* Left - Down */
        if ( !isTop ) total += (b[POS(x - 1, y - 1, g)] == ALIVE_CELL); /* Up - Left */
    }

    if ( !isBot ) total += (b[POS(x + 1, y, g)] == ALIVE_CELL); /* Down */
    if ( !isTop ) total += (b[POS(x - 1, y, g)] == ALIVE_CELL); /* Up  */

    return total;
}

/**
 * Private function which process a cell, i.e update the cell on the other board according to ome rules
 * %param x : Position on X of the cell on the board
 * %param y : Position on Y of the cell on the board
 * %param g : Game struct which contains all information relative to the game
 * %return  : New state of the cell in x / y coordinate.
 */
char __process(unsigned int x, unsigned int y, Game* g) {
    unsigned int neightbour = __neighbourCell(x, y, g);    
  
    fprintf(stderr, "total of neightbour : %d [%d, %d] - Board : %s\n", neightbour, x, y, g->board);
    if ( neightbour < 2 || neightbour > 3 ) return DEAD_CELL;
    else if ( neightbour == 3 )             return ALIVE_CELL;
    else                                    return g->board[POS(x, y, g)];
}

void processGameTick(Game *g) {
    int my_id, total_proc;
    unsigned int x, y;
    char* next;

    MPI_Comm_rank(MPI_COMM_WORLD,&my_id);
    MPI_Comm_size(MPI_COMM_WORLD,&total_proc);

    next = __newBoard(g->rows, g->cols);

    for ( x = (my_id != 0); x < g->cols - (my_id != total_proc - 1); x++ )
        for ( y = 0; y < g->rows; y++ ) 
            next[POS(x, y, g)] = __process(x, y, g);
  
    free(g->board);
    g->board = NULL;
    g->board = next;
}

Game* loadBoard(char* name) { 
    char reader = ' ';
    unsigned int rows = 0, cols = 0;
    FILE* fp = NULL;    
    Game *g = NULL;    

    if ( (fp = fopen(name, "r")) == NULL ) return NULL;
    if ( fscanf(fp, "Rows : %d\nCols : %d\n", &rows, &cols) != 2) { fclose(fp); return NULL; }

    g = newGame(rows, cols);

    DEBUG_MSG("Rows : %d, Cols : %d\n", rows, cols);
    rows = 0; cols = 0; /* Reinit variable */
    
    while ( (reader = fgetc(fp)) != EOF ) {
        if ( reader == '.' ) reader = DEAD_CELL;
        if ( reader == '#' ) reader = ALIVE_CELL;
        
        if ( reader == '\n') ++cols;
        else g->board[POS(cols, rows, g)] = reader;
        
        if ( ++rows > g->cols ) rows = 0; /* We are going to go over cols due to \n */
    }

    fclose(fp);

    if ( cols != g->cols && rows != g->rows ) { freeGame(g); return NULL; }
    return g;
}

bool saveBoard(Game *g) {
    unsigned int i;
    FILE *fp = NULL;

    if ( (fp = fopen("output.gol", "w")) == NULL ) return false;

    fprintf(fp, "Rows : %d\nCols : %d\n", g->rows, g->cols);
    for ( i = 0; i < g->cols * g->rows; i++ ) {

        fprintf(fp, "%c", ((g->board[i]) ? '#' : '.') );
        if ( i % g->cols == g->cols - 1 ) fprintf(fp, "\n"); 
    }

    #ifdef PRINT
        printf("File saved into : output.gol\n");
    #endif

    fclose(fp);
    return true;
}
