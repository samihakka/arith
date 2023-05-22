/**************************************************************
 *
 *                     cv_prepack.h
 *
 *     Assignment: CS40 HW4 arith
 *     Authors:  shakka01, cbolin01
 *     Date:     02/24/23
 *
 *     Interface of cv_prepack, which gives the client ability to 
 *     convert a from component video structs to luminance values
 *     structs, which contain values before discrete cosine transformation.
 *     Also contains functions that convert from luminance values to 
 *     component video. Functions that go from luminance values and average
 *     pb and prs, converting them to structs that are ready to be packed into
 *     32-bit codewords and vice versa.
 *
 **************************************************************/
#ifndef CV_PREPACK_INCLUDED
#define CV_PREPACK_INCLUDED

#include "a2blocked.h"
#include "a2methods.h"
#include "a2plain.h"
#include "assert.h"
#include "pnm.h"
#include "bitpack.h"
#include "arith40.h"
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern Pnm_ppm cv_to_lv(Pnm_ppm pixmap);
extern Pnm_ppm lv_to_prepack(Pnm_ppm pixmap);

extern Pnm_ppm prepack_to_lv(Pnm_ppm pixmap);
extern Pnm_ppm lv_to_cv(Pnm_ppm pixmap);


#endif