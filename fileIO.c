/**************************************************************
 *
 *                     fileIO.c
 *
 *     Assignment: CS40 HW4 arith
 *     Authors:  shakka01, cbolin01
 *     Date:     02/24/23
 *
 *     Implementation fileIO, contains all the functions 
 *     necessary for reading from a file and organizing
 *     the data into Pnm_ppm structs, as well as outputting
 *     data in the desired format, wether that be codeword
 *     output to standard output or a ppm_write in the 
 *     decompression process.
 *
 **************************************************************/
#include "fileIO.h"

static void apply_trim_ppm(int col, int row, A2Methods_UArray2 uarray2, 
                           void *elem, void *cl);
static void apply_print_codewords(int col, int row, A2Methods_UArray2 uarray2,
                           void *elem, void *cl);
static void singular_print_codeword(uint32_t bits);
static void apply_read_codewords(int col, int row, A2Methods_UArray2 uarray2, 
                           void *elem, void *cl);

/*    =============================================================    
      ====================== Compression ==========================    
      =============================================================    */

/* read_and_trim
 *       Purpose: if the image passed into this function has an uneven width
 *                or height, trim it into an even height/width and return
 *                the trimmed ppm
 *    Parameters: input: a file pointer to the file containing the image
 *  Expectations: the file pointer input is not null
 *       Returns: a ppm that has been trimmed, meaning it has an even width
 *                and height
 */
Pnm_ppm read_and_trim(FILE *input)
{
        assert(input != NULL);
        A2Methods_T methods = uarray2_methods_plain;
        assert(methods);
        A2Methods_mapfun *map = methods->map_row_major;
        assert(map);

        /* read in the image and set local variables */
        Pnm_ppm pixmap = Pnm_ppmread(input, methods);
        unsigned width       = pixmap->width;
        unsigned height      = pixmap->height;
        int size = methods->size(pixmap);
        
        bool needs_trim = false;
        
        /* if pixels has an odd width/height, trim w/h and change needs_trim */
        if (width % 2 != 0) {
                width -= 1;
                pixmap->width = width;
                needs_trim = true;
        } if (height % 2 != 0) {
                height -= 1;
                pixmap->height = height;
                needs_trim = true;
        }
        /* if it does need a trim, call the apply function on new array and
           free the old array */
        if (needs_trim) {
                A2Methods_UArray2 trimmed_map = methods->new(width, 
                                                             height, size);
                map(trimmed_map, apply_trim_ppm, pixmap);
                A2Methods_UArray2 orig = pixmap->pixels;
                pixmap->pixels = trimmed_map;
                methods->free(&orig);
        }
        return pixmap;
}

/* apply_trim_ppm
 *      Purpose: copies the pixels from an untrimmed ppm to a trimmed array
 *   Parameters: col: the x coordinate of the current pixel being rotated
 *               row: the y coordinate of the current pixel being rotated
 *               original_pixels: uarray2 that is the trimmed map to be filled
 *               elem: the element at a given column or row
 *               closure: a Pnm_ppm struct containing the pnm with untrimmed
 *                        array
 *  Expectations: uarray2 is not NULL
 *                0 <= col < width of the uarray2
 *                0 <= row < height of the uarray2
 *                closure is not NULL
 *      Returns: Nothing
 */
static void apply_trim_ppm(int col, int row, A2Methods_UArray2 uarray2, 
                           void *elem, void *cl) {
        Pnm_ppm           local_ppm = cl;
        A2Methods_UArray2 orig      = local_ppm->pixels;

        A2Methods_Object *pixel;
        pixel = (A2Methods_Object *)(local_ppm->methods->at(orig, col, row));

        /* Copy the pixel value from the original image to the rotated image */
        memcpy(elem, pixel, local_ppm->methods->size(orig));

        (void) uarray2;
}


/* print_codewords
 *       Purpose: Takes in a ppm and calls a map function to print the codewords
 *                to stdout.
 *    Parameters: cw_map is the ppm containing the codewords array
 *  Expectations: pnm_ppm is the pnm that contains the codeword array
 *       Returns: none
 */
void print_codewords(Pnm_ppm cw_map)
{
        assert(cw_map != NULL);
        A2Methods_mapfun *map = cw_map->methods->map_row_major;

        map(cw_map->pixels, apply_print_codewords, NULL);
}

/* apply_print_codewords
 *      Purpose: map through the entire array of codewords
 *   Parameters: col, row: coordinates of the codeword (unused)
 *               uarray2: unused
 *               elem: the spot in the uarray2 that we are currently at
 *               cl: pointer to a Pnm_ppm that holds unsigned ints array
 * Expectations: uarray2 is not NULL
 *               0 <= col < width of uarray2
 *               0 <= row < height of uarray2
 *      Returns: none
 */
static void apply_print_codewords(int col, int row, A2Methods_UArray2 uarray2, 
                           void *elem, void *cl) {

        /* get the pixel at this location in array, then pass it to singular
           print function */
        uint32_t *pixel = elem;
        singular_print_codeword(*pixel);
        
        (void) uarray2; (void) cl; (void) col; (void) row;
}

/* singular_print_codeword
 *       Purpose: Prints a singular codeword in big-endian order
 *    Parameters: bits, a 32 bit integer that is a codeword
 *  Expectations: none
 *       Returns: none, just prints the codeword to the file
 */
static void singular_print_codeword(uint32_t bits)
{

        /* extract 4 fields of singular bytes by shifting, then "and"ing 
           with a one byte container of all 1's */
        unsigned char c1 = (bits >> 24) & 0xFF;
        unsigned char c2 = (bits >> 16) & 0xFF;
        unsigned char c3 = (bits >> 8) & 0xFF;
        unsigned char c4 = bits & 0xFF;

        /* print each character to stdout in big endian order */
        putchar(c1);
        putchar(c2);
        putchar(c3);
        putchar(c4);
}


/*    =============================================================    
      ====================== Decompression ========================    
      =============================================================    */

/* read_codewords
 *       Purpose: maps over the input file
 *    Parameters: input: a file pointer to the file containing the image
 *  Expectations: the file pointer input is not null
 *       Returns: a ppm that has been trimmed, meaning it has an even width
 *                and height
 */
Pnm_ppm read_codewords(Pnm_ppm pixmap, FILE *in)
{ 
        assert(in != NULL);
        assert(pixmap != NULL);

        A2Methods_mapfun *map = pixmap->methods->map_row_major;
        map(pixmap->pixels, apply_read_codewords, in);
        
        return pixmap;
}


/* apply_read_codewords
 *      Purpose: Read in codewords from a file and puts them in codeword array
 *   Parameters: col, row: unused
 *               uarray2: unused
 *               elem: the spot in the uarray2 that we are currently at
 *               cl: pointer to the file that we are decompressing
 * Expectations: cl is not the end of file (we don't reach eof while reading)
 *               a codeword 
 *      Returns: none
 */
static void apply_read_codewords(int col, int row, A2Methods_UArray2 uarray2,
                           void *elem, void *cl)
{
        /* read 4 bytes, one byte at a time */
        unsigned char c1 = getc(cl);
        assert(!feof(cl));
        unsigned char c2 = getc(cl);
        assert(!feof(cl));
        unsigned char c3 = getc(cl);
        assert(!feof(cl));
        unsigned char c4 = getc(cl);
        assert(!feof(cl));

        /* convert bytes into a 32 bit representation */
        uint32_t uc1 = c1;
        uint32_t uc2 = c2;
        uint32_t uc3 = c3;
        uint32_t uc4 = c4;

        /* shift 8 byte quantities into their expected Codeword positions */
        uc1 <<= 24;
        uc2 <<= 16;
        uc3 <<= 8;

        /* combine the four single byte fields, add codeword to array */
        uint32_t codeword = uc1 | uc2 | uc3 | uc4;
        memcpy(elem, &codeword, sizeof(uint32_t));

        (void) uarray2; (void) col; (void) row;
}

/* print_ppmfile
 *       Purpose: print the finalized ppm and free the associated uarray2
 *    Parameters: input: a Pnm_ppm struct
 *  Expectations: the Pnm_ppm struct is not null
 *       Returns: none
 */
void print_ppmfile(Pnm_ppm pixmap)
{
        assert(pixmap != NULL);
        Pnm_ppmwrite(stdout, pixmap);
        pixmap->methods->free(&pixmap->pixels);
}