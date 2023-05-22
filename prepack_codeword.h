/**************************************************************
 *
 *                     prepack_codeword.h
 *
 *     Assignment: CS40 HW4 arith
 *     Authors:  shakka01, cbolin01
 *     Date:     02/24/23
 *
 *     Interface of prepack_codeword. The only two defined functions
 *     packing and unpacking bits. Packing is used in the compression
 *     process to assemble a struct of six integer values into a 
 *     singular 32 bit representation, and unpacking is the reverse 
 *     process of this.
 *
 **************************************************************/
#ifndef PREPACK_CODEWORD_INCLUDED
#define PREPACK_CODEWORD_INCLUDED

#include "a2blocked.h"
#include "a2methods.h"
#include "a2plain.h"
#include "assert.h"
#include "pnm.h"
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern Pnm_ppm    pack_bits(Pnm_ppm prepack_map);
extern Pnm_ppm    unpack_bits(Pnm_ppm bitpacked_map);

#endif
