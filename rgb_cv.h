/**************************************************************
 *
 *                     rgb_cv.h
 *
 *     Assignment: CS40 HW4 arith
 *     Authors:  shakka01, cbolin01
 *     Date:     02/24/23
 *
 *     Interface of rgb_cv, which gives the client ability to 
 *     convert a pnm_ppm containing rgb unsigned values into floats,
 *     and then those floats into component video structs containing
 *     y, pb, and pr values. Also contains functions that perform the
 *     opposite conversions.
 *
 **************************************************************/
#ifndef RGB_CV_INCLUDED
#define RGB_CV_INCLUDED

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

extern Pnm_ppm rgb_to_rgbf(Pnm_ppm pixmap);
extern Pnm_ppm rgbf_to_cv(Pnm_ppm pixmap);

extern Pnm_ppm cv_to_rgbf(Pnm_ppm pixmap);
extern Pnm_ppm rgbf_to_rgb(Pnm_ppm pixmap);

#endif