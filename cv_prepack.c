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
#include "cv_prepack.h"

/* holds y, pb, and pr values to convert from cv to luminance values & vv */
typedef struct Component_Video {
        float y;
        float pb;
        float pr;
} Component_Video;

/* contain values to be used for DCT to prepack structs or the other way */
typedef struct Luminance_Values {
        float y1;
        float y2;
        float y3;
        float y4;
        float avg_pb;
        float avg_pr;
} Luminance_Values;

/* these structs contain everything needed to be packed into codewords */
typedef struct PrePack {
        uint64_t a;
        int64_t b;
        int64_t c;
        int64_t d;
        unsigned index_pb;
        unsigned index_pr;
} PrePack;

/* used to convert between floats and ints for a, b, c, d values */
const float SCALE_A_F = 64.0;
const int SCALE_A_I = 64;
const float SCALE_BCD_F = 103.3;
const int SCALE_BCD_I = 103;

static Luminance_Values get_luminance(A2Methods_UArray2 cv_array,
                unsigned col, unsigned row);
static void apply_lv_to_prepack(int col, int row, A2Methods_UArray2 uarray2,
                            void *elem, void *cl);
static void apply_prepack_to_lv(int col, int row, A2Methods_UArray2 uarray2,
                            void *elem, void *cl);
static void apply_lv_to_cv(int col, int row, A2Methods_UArray2 orig,
                            void *elem, void *cl);
static void set_cv(Pnm_ppm cv_ppm, int col, int row, float y_val, Luminance_Values *lv_p) ;
unsigned Arith40_index_of_chroma(float chroma);
float    Arith40_chroma_of_index(unsigned n);
static float clamp(float val, float min, float max);


/* cv_to_lv
 *      Purpose: Convert all component video structs in a pixmap to 
 *      luminance values structs. Frees the old uarray holding cv's and 
 *               returns pixmap with lv structs.
 *   Parameters: A Pnm_ppm that contains the component video pixmap
 * Expectations: The pixmap is valid (not a null Pnm_ppm)
 *      Returns: A pixmap with each pixel in luminance value form
 */
Pnm_ppm cv_to_lv(Pnm_ppm pixmap)
{
        assert(pixmap != NULL);
        unsigned width = pixmap->width;
        unsigned height = pixmap->height;

        A2Methods_UArray2 lv_array = pixmap->methods->new(width / 2, \
                     height / 2, sizeof(Luminance_Values));

        unsigned lv_row, lv_col; /* coords for lv_array of 1/2 size of cv arr */
        /* loop through 2x2 blocks of pixels of component video array */
        for (unsigned row = 0; row < height; row += 2) {
                lv_row = row / 2;
                for (unsigned col = 0; col < width; col += 2) {
                        lv_col = col / 2;
                        /* obtain luminance values from cv_values struct */
                        Luminance_Values add_lv = get_luminance(pixmap->pixels,
                                                  col, row);
                        /* get index to put lv in and insert it */
                        Luminance_Values *lv_index = \
                                pixmap->methods->at(lv_array, lv_col, lv_row);
                        memcpy(lv_index, &add_lv, sizeof(Luminance_Values));
                }
        }

        /* exchange pixmap's pixels and free the old map, also 
           cutting width and height in half */
        A2Methods_UArray2 to_free = pixmap->pixels;
        pixmap->pixels = lv_array;
        pixmap->width /= 2;
        pixmap->height /= 2;
        pixmap->methods->free(&to_free);
        return pixmap;
}


/* get_luminance
 *      Purpose: Given an array of component video structs, calculate
 *               the luminance values (a, b, c, d, and avg_pb/r).
 *               Also clamps these values into an acceptable range.
 *   Parameters: col, row: index of cv array that we want to convert
 * Expectations: the cv array is not null and row & col in range
 *      Returns: a luminance value struct containing a, b, c, d, and avg_pb/r
 */
static Luminance_Values get_luminance(A2Methods_UArray2 cv_array,
                unsigned col, unsigned row)
{
        A2Methods_T local_methods = uarray2_methods_plain;
        Luminance_Values to_insert;
        
        /* initialize luminance values from component video y values & clamp */
        Component_Video *c1 = local_methods->at(cv_array, col, row);
        to_insert.y1 = clamp(c1->y, 0, 1);
        Component_Video *c2 = local_methods->at(cv_array, col + 1, row);
        to_insert.y2 = clamp(c2->y, 0, 1);
        Component_Video *c3 = local_methods->at(cv_array, col, row + 1);
        to_insert.y3 = clamp(c3->y, 0, 1);
        Component_Video *c4 = local_methods->at(cv_array, col + 1, row + 1);
        to_insert.y4 = clamp(c4->y, 0, 1);

        /* calculate and initialize average pb and pr, clamping them as well */
        to_insert.avg_pb = clamp(((c1->pb + c2->pb + c3->pb + c4->pb) / 4.0),
                                                                   -0.5, 0.5);
        to_insert.avg_pr = clamp(((c1->pr + c2->pr + c3->pr + c4->pr) / 4.0),
                                                                   -0.5, 0.5);
        return to_insert;
}


/* lv_to_prepack
 *      Purpose: Convert all luminance value structs in a pixmap to 
 *      PrePack structs. Frees the old uarray holding lv's and 
 *               returns pixmap with PrePack structs.
 *   Parameters: A Pnm_ppm that contains the luminance value pixmap
 * Expectations: The pixmap is valid (not a null Pnm_ppm)
 *      Returns: A pixmap with each pixel in PrePack form
 */
Pnm_ppm lv_to_prepack(Pnm_ppm pixmap)
{
    A2Methods_T methods = uarray2_methods_plain;
    A2Methods_mapfun *map = methods->map_row_major;

        /* create new array and map it, performing DCT and populating
           new array with calculated values */
        unsigned width = pixmap->methods->width(pixmap->pixels);
        unsigned height = pixmap->methods->height(pixmap->pixels);
        A2Methods_UArray2 prepack_array = pixmap->methods->new(width, \
                                    height, sizeof(PrePack));
        map(prepack_array, apply_lv_to_prepack, pixmap->pixels);
    
        /* rearrange pixmap->pixels and free unused array */
        A2Methods_UArray2 to_free = pixmap->pixels;
        pixmap->pixels = prepack_array;
        pixmap->methods->free(&to_free);
        return pixmap;
}


/* apply_lv_to_prepack
 *      Purpose: Convert all luminance values in a pixmap to prepack structs,
 *               which are ready to be exported into codewords.
 *               Frees the old uarray holding lum_values and returns
 *               pixmap of prepacked values
 *   Parameters: col, row: coordinates of the pixel
 *               uarray2: unused
 *               elem: the spot in the uarray2 that we are currently at
 *               cl: pointer to a Pnm_ppm that holds lum_values
 * Expectations: original_pixels is not NULL
 *               0 <= col < width of orig
 *               0 <= row < height of orig
 *               closure is not NULL 
 *      Returns: none
 */
static void apply_lv_to_prepack(int col, int row, A2Methods_UArray2 uarray2,
                            void *elem, void *cl)
{       A2Methods_T methods = uarray2_methods_plain;
        A2Methods_UArray2 orig      = cl;
        /* pointer to luminance value struct */
        Luminance_Values *lv = (A2Methods_Object *)(methods->at(orig, 
                                                                col, row));
        
        PrePack temp;
        /* calculate and clamp DCT values into respective ranges */
        float a = clamp(((lv->y4 + lv->y3 + lv->y2 + lv->y1) / 4.0), 0.0, 1.0);
        float b = clamp(((lv->y4 + lv->y3 - lv->y2 - lv->y1) / 4.0), -0.3, 
                                                                      0.3);
        float c = clamp(((lv->y4 - lv->y3 + lv->y2 - lv->y1) / 4.0), -0.3, 
                                                                      0.3);
        float d = clamp(((lv->y4 - lv->y3 - lv->y2 + lv->y1) / 4.0), -0.3, 
                                                                      0.3);
        
        /* set values and make them into the appropriate number of bits */
        temp.a = floor(SCALE_A_I * a);
        temp.b = SCALE_BCD_I * b;
        temp.c = SCALE_BCD_I * c;
        temp.d = SCALE_BCD_I * d;

        /* perform index of chroma on average pb and pr values */
        temp.index_pb = Arith40_index_of_chroma(lv->avg_pb);
        temp.index_pr = Arith40_index_of_chroma(lv->avg_pr);

        memcpy(elem, &temp, sizeof(PrePack));

        (void)uarray2;
}

/* prepack_to_lv
 *      Purpose: Convert all PrePack video structs in a pixmap to 
 *      luminance values structs. Frees the old uarray holding PrePacks and
 *               returns pixmap with lv structs.
 *   Parameters: A Pnm_ppm that contains the PrePack struct pixmap
 * Expectations: The pixmap is valid (not a null Pnm_ppm)
 *      Returns: A pixmap with each pixel in luminance value form
 */
Pnm_ppm prepack_to_lv(Pnm_ppm pixmap)
{
        A2Methods_mapfun *map = pixmap->methods->map_row_major;
        unsigned width = pixmap->methods->width(pixmap->pixels);
        unsigned height = pixmap->methods->height(pixmap->pixels);
        
        /* create new array and map it, performing inverse DCT and populating
           new array with calculated values */
        A2Methods_UArray2 lv_array = pixmap->methods->new(width, 
                                     height, sizeof(Luminance_Values));
        map(lv_array, apply_prepack_to_lv, pixmap);
        
        /* rearrange pixmap->pixels and free unused array */
        A2Methods_UArray2 to_free = pixmap->pixels;
        pixmap->pixels = lv_array;
        pixmap->methods->free(&to_free);
        
        return pixmap;
}


/* apply_prepack_to_lv
 *      Purpose: Convert all Pnm_rgbs in a pixmap from PrePack structs
 *               to luminance value structs.
 *               Frees the old uarray holding unsigned ints and returns
 *               pixmap in float form
 *   Parameters: col, row: coordinates of the pixel
 *               uarray2: unused
 *               elem: the spot in the uarray2 that we are currently at
 *               cl: pointer to a Pnm_ppm that holds unsigned ints array
 * Expectations: original_pixels is not NULL
 *               0 <= col < width of orig
 *               0 <= row < height of orig
 *               closure is not NULL 
 *      Returns: none
 */
static void apply_prepack_to_lv(int col, int row, A2Methods_UArray2 uarray2,
                            void *elem, void *cl)
{
        Pnm_ppm local_ppm = cl;

        A2Methods_UArray2 orig      = local_ppm->pixels;

        PrePack *pp = (A2Methods_Object *)(local_ppm->methods->at(orig,
                                                        col, row));
    
        Luminance_Values lv;

        /* scales ints to floats */
        float a = pp->a / SCALE_A_F;
        float b = pp->b / SCALE_BCD_F;
        float c = pp->c / SCALE_BCD_F;
        float d = pp->d / SCALE_BCD_F;

        /* performs inverse of DCT */
        lv.y1 = a - b - c + d;
        lv.y2 = a - b + c - d;
        lv.y3 = a + b - c - d;
        lv.y4 = a + b + c + d;

        /* quantizers from chroma back to average pb/pr */
        lv.avg_pb = Arith40_chroma_of_index(pp->index_pb);
        lv.avg_pr = Arith40_chroma_of_index(pp->index_pr);

        memcpy(elem, &lv, sizeof(Luminance_Values));

        (void)uarray2;
}


/* lv_to_cv
 *      Purpose: Convert an array of luminance values into an array
 *               of component video structs. The new array will be
 *               4x the size of the lum_values array
 *   Parameters: A Pnm_ppm that contains the luminance value pixmap
 * Expectations: The pixmap is valid (not a null Pnm_ppm)
 *      Returns: A pixmap with each pixel in component video form
 */
Pnm_ppm lv_to_cv(Pnm_ppm pixmap)
{
        assert(pixmap != NULL);
        A2Methods_mapfun *map = pixmap->methods->map_row_major;

        /* double width and height of pixmap in preparation for larger map */
        pixmap->width *= 2;
        pixmap->height *= 2;

        /* declare and set component video array as pixmap's pixels */
        A2Methods_UArray2 cv_array = pixmap->methods->new(pixmap->width, \
                                     pixmap->height, sizeof(Component_Video));
        A2Methods_UArray2 lv_array = pixmap->pixels; /* store lum array */
        pixmap->pixels = cv_array;
        
        /* map to populate the component video array */
        map(lv_array, apply_lv_to_cv, pixmap);
        
        /* free the unused array and return newly populated pixmap */
        pixmap->methods->free(&lv_array);
        return pixmap;
}


/* apply_lv_to_cv
 *      Purpose: Convert all Pnm_rgbs in a pixmap from luminance_val structs
 *               to component video structs.
 *   Parameters: col, row: coordinates of the luminance value struct
 *               uarray2: the array that holds luminance values
 *               elem: the spot in the uarray2 that we are currently at
 *               cl: pointer to an empty array
 * Expectations: closure is not NULL
 *               0 <= col < width of lum_values
 *               0 <= row < height of lum_values
 *      Returns: none
 */
static void apply_lv_to_cv(int col, int row, A2Methods_UArray2 uarray2,
                            void *elem, void *cl)
{
        /* because cv array is 4x the size of luminance value array */
        col = col * 2;
        row = row * 2;
        Pnm_ppm cv_ppm = cl;

        Luminance_Values *lv_p = elem;
        
        /* set component video structs for all 4 pixels in a 2x2 block */
        set_cv(cv_ppm, col, row, lv_p->y1, lv_p);
        set_cv(cv_ppm, col + 1, row, lv_p->y2, lv_p);
        set_cv(cv_ppm, col, row + 1, lv_p->y3, lv_p);
        set_cv(cv_ppm, col + 1, row + 1, lv_p->y4, lv_p);

        (void) uarray2;
}


/* set_cv
 *      Purpose: Given a ptr to a luminance value, set_cv sets the component
 *               video struct at the specified col and row in the component
 *               video array
 *   Parameters: cv_ppm: the ppm that contains the array we are inserting into
 *               col, row: index of the cv_ppm to insert to
 *               y_val: will be y1/2/3/4 depending on what is passed in
 *               lv_p: pointer to a luminance value struct
 * Expectations: cv_ppm is valid, col and row in bounds, pointer not null
 *      Returns: none, but updates the component video array that is contained
 *               in cv_ppm
 */
static void set_cv(Pnm_ppm cv_ppm, int col, int row, float y_val, Luminance_Values *lv_p) 
{
        assert(cv_ppm != NULL);
        Component_Video to_insert;
        /* set values of component video to insert */
        to_insert.y = y_val;
        to_insert.pb = lv_p->avg_pb;
        to_insert.pr = lv_p->avg_pr;

        /* get the index to insert into and insert */
        A2Methods_Object *writing_cell;
        writing_cell = (A2Methods_Object *)(cv_ppm->methods->at(
                                        cv_ppm->pixels, col, row));
        memcpy(writing_cell, &to_insert, sizeof(Component_Video));
}


/* clamp
 *      Purpose: Clamp specified value between given min and maxes
 *   Parameters: val: the float to be clamped
 *               min: minimum value for the val to be
 *               max: maximum value for the val to be
 * Expectations: none
 *      Returns: if val was between min and max, val. If not, it returns
 *               a clamped value that is either min or max, depending on
 *               which extrema it was on.
 */
static float clamp(float val, float min, float max) 
{
        if (val < min) {
                return min;
        }
        else if (val > max) {
                return max;
        }
        else {
                return val;
        }
}