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
/* FICHIER : matrix.h                                        */
/*---------------------------------------------------------- */

#ifndef MATRIX_H
#define MATRIX_H

#include "game_struct.h"

/**
 * Private function which return a sub-matrix according to a start on x and y and slice size
 * %param src : Source matrix which we extract data
 * %param startx : Where to start on x
 * %param starty : Where to start on y
 * %param xslice_size : Slice size on x
 * %param yslice_size : Slice size on y
 * %return : The submatrix of size xslice_size * yslice_size starting at startx, starty
 */
Game *__subMatrix(Game *src, int startx, int starty, int xslice_size, int yslice_size);

/**  
 * Merge a submatrix with a destination matrix
 * %param src : Source matrix 
 * %param dest : Destination matrix
 * %param startx : Where to start on X in dest matrix
 * %param starty : Where to start merge in dest matrix
 */
void __mergeMatrix(Game *src, Game *dest, int startx, int starty);

/**
 * Big function that share all border to other process, share each needed part to neightbour and get other border
 * %param s : Game where the border need to go and need to be shared
 * %param my_x : My process id on x
 * %param my_y : My process id on y 
 * %param slice_size : Size of the slice into the first big matrix
 * %param proc_slice : Number of proc on the columns or rows
 */
void shareMatrixBorder(Game *s, int my_x, int my_y, int slice_size, int proc_slice );

/** 
 * Gather all submatrix into the original one (after the compute ), this function MUST be done by all process
 * %param g : Game where the final grid will be
 * %param s : Submatrix to send
 * %param my_x : My process id on x
 * %param my_y : My process id on y
 * %param slice_size : Size of the slice of the principale board
 * %param proc_slice : Total number of process on a column or a row
 * %param total_proc : total number of process 
 */
void gatherMatrix(Game *g, Game *s, int my_x, int my_y, int slice_size, int proc_slice, int total_proc);


/* Schematix of a matrix split into submatrix */
/*  Matrix : 3 x 3, Np = 9 
 *  [V][S] [S][V][S] [S][V] < x: 0
 *  [S][S] [S][S][S] [S][S]  
 *
 *  [S][S] [S][S][S] [S][S] < x: 1
 *  [V][S] [S][V][S] [S][V] < 
 *  [S][S] [S][S][S] [S][S] < 
 *
 *  [S][S] [S][S][S] [S][S] < x: 2
 *  [V][S] [S][V][S] [S][V] < 
 *   y: 0    y: 1     y: 2
 *  
 *  S = Buffer   
 *  V = Value of original 3 x 3 matrix  
 */
/** 
 * Function which send matrix to other process
 * %param g : Original game board to split 
 * %param slice_size : Size of the slice of the submatrix
 * %param proc_slice : Total number of processus by rows or columns
 * %return : SubMatrix of process 0
 */
 
Game* sendAllSubMatrice(Game *g, int slice_size, int proc_slice);

/**
 * Received a matrix send by process 0
 * %param my_x : My processus id on x
 * %param my_y : My processus id on y
 * %param slice_size : Size of a slice of the principal grid
 * %param proc_slice : Total number of proc on a rows or column
 * %return : return the board including the offset for border sharing
 */
Game* receivedMatrix(int my_x, int my_y, int slice_size, int proc_slice);

#endif
