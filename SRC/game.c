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
    return g->cols * y + x;
}

/**
 * Private function that print a simple line
 * %param g : Game structure which contains the information relative to the game
 * %param pf : Pointer to a print function
 */
void __printLine(Game* g, int (*pf)(const char *, ...)) {
    unsigned int i = 0;

    (*pf)("+");
    for ( i = 0; i < g->cols + 2; i++ ) /* the 2 '+'  */
        (*pf)("-");
    (*pf)("+\n");

}

/**
 * Private function that really print the board contenant 
 * %param g : Game struct which contains the board to print
 * %param pf : Pointer to a printing function
 */
void __gamePrint (Game* g, int (*pf)(const char *, ...)) {
    unsigned int x, y;

    if ( *pf == printw ) /* If we use ncurses we need to replace cursor */
        move(0, 0);
    
    (*pf)("Board size : \n");
    (*pf)("  %d Columns\n", g->cols);
    (*pf)("  %d rows\n", g->rows);

    __printLine(g, pf);

    for ( y = 0; y < g->rows; y++) {
        (*pf)("| ");
        for ( x = 0; x < g->cols; x++) {
            (*pf)("%c", ((g->current_board[POS(x, y, g)] == DEAD_CELL) ? '.' : '#'));    
        }
        
        (*pf)(" |\n");
    }
    
    __printLine(g, pf);
    
    if ( *pf == printw ) /* If we use ncurses we need to refresh the display */
        refresh();

    DEBUG_MSG("Print board finish\n");
}

void swapGrid(Game* g) {
    char *tmp = g->current_board;
    g->current_board = g->next_board;
    g->next_board = tmp;
}

void gamePrintInfo(Game* g, Option o) {
    #ifndef PRINT
         return;
    #endif

    int (*printFunc)(const char*, ...);    
    printFunc = ( o.use_ncurses ) ? &printw : &printf; 

    if ( o.max_tick >= 0 )
        printFunc("%d Generation left.\n", o.max_tick);
    
    __gamePrint(g, printFunc);
}

/** 
 * Private function that allocate a new board
 * %param rows : Total number of rows onto the board
 * %param cols : Total number of column onto the board
 * %return     : Allocated array of char which will contains the board
 */
char* __newBoard(unsigned int rows, unsigned int cols) {
    char* board = NEW_ALLOC_K(rows * cols, char);
    return board;
}

/**
 * Private function that create a new game
 * %param rows : Total number of rows onto the new board
 * %param cols : Total number of Column onto the new board
 * %return     : Allocated game structure which contains all the information 
 */
Game* __newGame(unsigned int rows, unsigned int cols) {
    Game* g = NEW_ALLOC(Game);
    
    g->rows = rows;
    g->cols = cols;
    
    g->current_board = __newBoard(rows, cols);
    g->next_board = __newBoard(rows, cols);

    return g;
}

void freeGame(Game* g)  {
    if ( g == NULL ) 
        return;

    free(g->current_board);
    free(g->next_board);
    free(g);
}

Game* generateRandomBoard(Option o) {

    unsigned int rows = 0, cols = 0;
    Game* g;
        
    g = __newGame(o.rows, o.cols);
    
    DEBUG_MSG("Ligne : %d, Cols : %d\n", o.rows, o.cols);
    for (rows = 0; rows < g->rows; rows++)
        for(cols = 0; cols < g->cols; cols++)
            g->current_board[POS(cols, rows, g)] = (
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
    char *b = g->current_board;

    if ( x % g->cols != g->cols - 1) {
        total += b[POS(x + 1, y,     g)]; /* Right */
        if ( y < g->rows - 1 ) total += b[POS(x + 1, y + 1, g)]; /* Right - Down */
        if ( y > 0 )           total += b[POS(x + 1, y - 1, g)]; /* Up - Right */
    }

    if ( x % g->cols != 0 ) { 
        total += b[POS(x - 1, y    , g)]; /* Left */
        if ( y < g->rows - 1 ) total += b[POS(x - 1, y + 1, g)]; /* Left - Down */
        if ( y > 0 )           total += b[POS(x - 1, y - 1, g)]; /* Up - Left */
    }

    if ( y < g->rows - 1 ) total += b[POS(x    , y + 1, g)]; /* Down */
    if ( y > 0 )           total += b[POS(x    , y - 1, g)]; /* Up  */

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

    if ( neightbour < 2 || neightbour > 3 ) return DEAD_CELL;
    else if ( neightbour == 3 )             return ALIVE_CELL;
    else                                    return g->current_board[POS(x, y, g)];
}

void gameTick(Game *g, Task* t) {

    unsigned int x, y;
    
    for (y = 0; y < g->rows; y++)
        for(x = t->min; x <= t->max; x++)
            g->next_board[POS(x, y, g)] = __process(x, y, g);

    DEBUG_MSG("Game tick finish");
}

Game* loadBoard(char* name) { 
    char reader = ' ';
    unsigned int rows = 0, cols = 0;
    FILE* fp = NULL;    
    Game *g = NULL;    

    if ( (fp = fopen(name, "r")) == NULL ) return NULL;
    if ( fscanf(fp, "Rows : %d\nCols : %d\n", &rows, &cols) != 2) { fclose(fp); return NULL; }

    g = __newGame(rows, cols);

    DEBUG_MSG("Rows : %d, Cols : %d\n", rows, cols);
    rows = 0; cols = 0; /* Reinit variable */
    
    while ( (reader = fgetc(fp)) != EOF ) {
        if ( reader == '.' ) reader = DEAD_CELL;
        if ( reader == '#' ) reader = ALIVE_CELL;
        
        if ( reader == '\n') ++rows;
        else g->current_board[POS(cols, rows, g)] = reader;
        
        if ( ++cols > g->cols ) cols = 0; /* We are going to go over cols due to \n */
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

        fprintf(fp, "%c", ((g->current_board[i]) ? '#' : '.') );
        if ( i % g->cols == g->cols - 1 ) fprintf(fp, "\n"); 
    }

    #ifdef PRINT
        printf("File saved into : output.gol\n");
    #endif

    fclose(fp);
    return true;
}
