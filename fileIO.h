/**************************************************************
 *
 *                     fileIO.h
 *
 *     Assignment: CS40 HW4 arith
 *     Authors:  shakka01, cbolin01
 *     Date:     02/24/23
 *
 *     Interface of fileIO, which on compression reads in ppm images, 
 *     trims them if necessary, and then outputs the image in codewords.
 *     When decompressing, it reads in codewords and prints a ppm
 *     image to stdout.
 *
 **************************************************************/
#ifndef FILEIO_INCLUDED
#define FILEIO_INCLUDED

#include "a2blocked.h"
#include "a2methods.h"
#include "a2plain.h"
#include "assert.h"
#include "pnm.h"
#include "bitpack.h"
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern Pnm_ppm read_and_trim(FILE *input);
extern void print_codewords(Pnm_ppm pixmap);
extern Pnm_ppm read_codewords(Pnm_ppm pixmap, FILE *in);
extern void print_ppmfile(Pnm_ppm pixmap);

#endif