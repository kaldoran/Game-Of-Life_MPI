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

/*---------------------------------------------------------- */
/* AUTEUR : REYNAUD Nicolas                                  */
/* FICHIER : game.h                                          */
/*---------------------------------------------------------- */


#ifndef GAME_H
#define GAME_H

#include "game_struct.h"
#include "option_struct.h"

#include "task_pile_struct.h"

/**
 * First need to define all the constante
 * thoses one are usefull for generate a random board if needed
 */
#define MIN_COLS_SIZE 5 /* Minimum number of cols */
#define MIN_ROWS_SIZE 3 /* Minimum number of rows */

#define POURCENT_BEEN_ALIVE 15 /* Pourcentage of cell to keep alive during generation */

#define DEAD_CELL 0
#define ALIVE_CELL 1

/**
 * Given X, and Y this function output the position into the board. 
 * For example POS(0,0,G) return 0, cause the cell in 0 on X, and 0 on Y is the cell 0 of the board
 * %param X : Position on the X coordinate
 * %param Y : Position on the Y coordinate
 * %param G : Board on which we need to compute the position
 * %return  : The associate position on the board
 */
#define POS(X, Y, G) (__position(X,Y,G))

/**
 * Function that print the board, this function determine if we need to print it or not
 * i.e if the programme is make with make display
 * This function also determine which function we need to use to display the board, and print the 
 * number of generation left.
 * 
 * %param g : The game which contains the board to print
 * %param o : Option which include the use_ncurses option
 */
void gamePrintInfo ( Game* g, Option o);

/**
 * Function that free the memory associate with a game
 * %param g : Game to free
 */
void freeGame(Game* g);

/**
 * Function that generate a random board if no are given
 * %param o : Option for generating the board
 * %return  : a random board
 */
Game* generateRandomBoard(Option o);

/**
 * Function that make the game tick, i.e function that iterate through the game board
 * and complet the other board.
 * %param g : game with contains the board on which we need to iterate
 * %param t : task that need to be done on this board
 */
void gameTick(Game *g, Task *t);

/**
 * Private function that is used in the main program
 * This function swap the 2 board of the game
 * %param g : Game which contains the 2 board to swap
 */
void swapGrid(Game* g);

/**
 * Load in memory a game / board contains into a file
 * %param name : path to the file to load
 * %return     : The game structure associate with the contenant of the file
 *               Or NULL if that fail [i.e the file is not valide
 */ 
Game* loadBoard(char* name);

/**
 * Function that save a game into a file
 * %param g : the board to save
 * %return  : true if it succeed 
 *            false otherwise
 */
bool saveBoard(Game *g);

#endif
