/**************************************************************
 *
 *                     compress40.c
 *
 *     Assignment: CS40 HW4 arith
 *     Authors:  shakka01, cbolin01
 *     Date:     02/24/23
 *
 *     Implementation of compress40, which contains a function to
 *     compress an image to about 1/3 of its original size and a function
 *     that decompresses images back to their original forms of p6 ppms.
 *
 **************************************************************/
#include "a2blocked.h"
#include "a2methods.h"
#include "a2plain.h"
#include "assert.h"
#include "pnm.h"
#include "bitpack.h"
#include "fileIO.h"
#include "compress40.h"
#include "rgb_cv.h"
#include "cv_prepack.h"
#include "prepack_codeword.h"
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const float COMP_DENOMINATOR = 255; /* this is the denominator of choice */
const int PNM_RGB_SIZE = 12; /* size of pnm_rgb struct */

#define header_fmt "COMP40 Compressed image format 2\n%u %u"

/*****************************************************************
 *                  Function Declarations                        *
 *****************************************************************/

/* compress40
 *      Purpose: Given a pointer to a file that contains an image, compresses 
 *               that image into 32 bit codewords and prints them to stdout.
 *   Parameters: input: pointer to a file that contains a ppm image
 * Expectations: input is not null
 *      Returns: none, but prints codewords to stdout (compressed image)
 */
void compress40(FILE *input)
{
    assert(input != NULL);
    A2Methods_T methods = uarray2_methods_plain;
    assert(methods);
    A2Methods_mapfun *map = methods->map_row_major;
    assert(map);
    /* I/O */
    Pnm_ppm pixmap = read_and_trim(input);

    /* rgb_cv */
    Pnm_ppm rgbf_map = rgb_to_rgbf(pixmap);
    Pnm_ppm cv_map = rgbf_to_cv(rgbf_map);

    /* cv_prepack */
    Pnm_ppm lum_map = cv_to_lv(cv_map);
    Pnm_ppm prepack_map = lv_to_prepack(lum_map);

    /* prepack_codeword */
    Pnm_ppm to_print = pack_bits(prepack_map);

    /* print the header */
    fprintf(stdout, header_fmt, to_print->width * 2, to_print->height * 2);
    fprintf(stdout, "\n");

    /* print codewords and free the pixmap */
    print_codewords(to_print);
    Pnm_ppmfree(&to_print);
}


/* decompress40
 *      Purpose: Given a pointer to a file that contains compressed image, 
 *               decompresses that image and then prints the proper ppm
 *               image to stdout.
 *   Parameters: input: pointer to a file that contains a compressed image
 * Expectations: input is not null
 *      Returns: none, but prints image to stdout (decompressed image)
 */
void decompress40(FILE *input)
{
    assert(input != NULL);
    A2Methods_T methods = uarray2_methods_plain;
    assert(methods);
    A2Methods_mapfun *map = methods->map_row_major;
    assert(map);

    /* read the width and height from header, take in the newline as well */
    unsigned height, width;
    int read = fscanf(input, "COMP40 Compressed image format 2\n%u %u", &width, 
                      &height);
    assert(read == 2);
    int c = getc(input);
    assert(c == '\n');

    /* initialize empty array */
    A2Methods_UArray2 empty = methods->new(width / 2, height / 2, PNM_RGB_SIZE);

    /* pixmap to be populated */
    struct Pnm_ppm pixmap = {.width = width / 2, .height = height / 2, 
        .denominator = COMP_DENOMINATOR, .pixels = empty, .methods = methods};

    /* fileIO */
    Pnm_ppm codewords = read_codewords(&pixmap, input);

    /* prepack_codeword */
    Pnm_ppm prepacked_map = unpack_bits(codewords);

    /* cv_prepack */
    Pnm_ppm lv_map = prepack_to_lv(prepacked_map);
    Pnm_ppm cv_map = lv_to_cv(lv_map);

    /* rgb_cv */
    Pnm_ppm rgbf_map = cv_to_rgbf(cv_map);
    Pnm_ppm rgb_map = rgbf_to_rgb(rgbf_map);

    /* write the pixmap to stdout */
    print_ppmfile(rgb_map);
}