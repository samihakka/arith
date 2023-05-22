/**************************************************************
 *
 *                     rgb_cv.c
 *
 *     Assignment: CS40 HW4 arith
 *     Authors:  shakka01, cbolin01
 *     Date:     02/24/23
 *
 *     Implementation of rgb_cv, which converts from rgb unsigned integers
 *     contained in a pnm_ppm into floats and then into component video
 *     structs. It also performs the opposite conversion from component
 *     video structs to rgb floats and then floats to unsigned integers.
 *
 **************************************************************/
#include "rgb_cv.h"

/* struct to hold rgb values in float form */
typedef struct float_rgb {
        float r;
        float g;
        float b;
} float_rgb;

/* holds y, pb, and pr values gathered from rgb float structs or vice versa */
typedef struct Component_Video {
        float y;
        float pb;
        float pr;
} Component_Video;

const float DENOMINATOR = 255; /* this is the denominator of choice */

static void apply_rgb_to_rgbf(int col, int row, A2Methods_UArray2 uarray2,
                               void *elem, void *cl);
static float_rgb singular_rgb_to_rgbf(Pnm_rgb pixel, float img_denominator);
static void apply_rgbf_to_cv(int col, int row, A2Methods_UArray2 uarray2,
                               void *elem, void *cl);
static void apply_cv_to_rgbf(int col, int row, A2Methods_UArray2 uarray2,
                                void *elem, void *cl);
static void apply_rgbf_to_rgb(int col, int row, A2Methods_UArray2 uarray2,
                                void *elem, void *cl);
static float clamp(float val, float min, float max);


/* rgb_to_rgbf
 *      Purpose: Convert all Pnm_rgbs in a pixmap from unsigned int to floats.
 *               Frees the old uarray holding unsigned ints and returns
 *               pixmap in float form
 *   Parameters: A Pnm_ppm that contains the original unsigned pixmap
 * Expectations: The pixmap is valid (not a null Pnm_ppm)
 *      Returns: A pixmap with all pixels in float form
 */
Pnm_ppm rgb_to_rgbf(Pnm_ppm pixmap)
{
        assert(pixmap != NULL);
        A2Methods_mapfun *map = pixmap->methods->map_row_major;
    
        /* create the new array and map to convert rgb unsigned to floats */
        A2Methods_UArray2 rgb_float_array = pixmap->methods->new( \
                          pixmap->width, pixmap->height, sizeof(float_rgb));
        map(rgb_float_array, apply_rgb_to_rgbf, pixmap);

        /* free the unused array, set the new array to pixmap's pixels */
        A2Methods_UArray2 to_free = pixmap->pixels;
        pixmap->pixels = rgb_float_array;
        pixmap->methods->free(&to_free);
        return pixmap;
}


/* rgbf_to_cv
 *      Purpose: Convert all Pnm_rgbs in a pixmap from floats to component
 *               video structs.Frees the old uarray holding floats and 
 *               returns pixmap with cv structs.
 *   Parameters: A Pnm_ppm that contains the rgb_floats pixmap
 * Expectations: The pixmap is valid (not a null Pnm_ppm)
 *      Returns: A pixmap with each pixel in component video form
 */
Pnm_ppm rgbf_to_cv(Pnm_ppm pixmap)
{
        assert(pixmap != NULL);
        A2Methods_mapfun *map = pixmap->methods->map_row_major;
        
        /* create the new array and map to convert rgb floats to component
           video structs */
        A2Methods_UArray2 cv_float_array = pixmap->methods->new(pixmap->width,
                                     pixmap->height, sizeof(Component_Video));
        map(cv_float_array, apply_rgbf_to_cv, pixmap);

        /* free the unused array, set the new array to pixmap's pixels */
        A2Methods_UArray2 to_free = pixmap->pixels;
        pixmap->pixels = cv_float_array;
        pixmap->methods->free(&to_free);
        return pixmap;
}

/* cv_to_rgbf
 *      Purpose: Convert all component in a pixmap holding video structs to
 *               rgb_floats. Frees the old uarray holding cv structs and 
 *               returns pixmap with rgb_floats.
 *   Parameters: A Pnm_ppm that contains the component video pixmap
 * Expectations: The pixmap is valid (not a null Pnm_ppm)
 *      Returns: A pixmap with each pixel in rgb_float form
 */
Pnm_ppm cv_to_rgbf(Pnm_ppm pixmap)
{
        assert(pixmap != NULL);
        A2Methods_mapfun *map = pixmap->methods->map_row_major;

        /* create the new array and map to convert component video structs
           to rgb floats */
        A2Methods_UArray2 rgb_float_array = pixmap->methods->new
                        (pixmap->width, pixmap->height, sizeof(float_rgb));
        map(rgb_float_array, apply_cv_to_rgbf, pixmap);

        /* free the unused array, set the new array to pixmap's pixels */
        A2Methods_UArray2 to_free = pixmap->pixels;
        pixmap->pixels = rgb_float_array;
        pixmap->methods->free(&to_free);
        return pixmap;
}


/* rgbf_to_cv
 *      Purpose: Convert all Pnm_rgbs in a pixmap from floats to component
 *               video structs.Frees the old uarray holding floats and 
 *               returns pixmap with cv structs.
 *   Parameters: A Pnm_ppm that contains the rgb_floats pixmap
 * Expectations: The pixmap is valid (not a null Pnm_ppm)
 *      Returns: A pixmap with each pixel in component video form
 */
Pnm_ppm rgbf_to_rgb(Pnm_ppm pixmap)
{
        assert(pixmap != NULL);
        A2Methods_mapfun *map = pixmap->methods->map_row_major;
        
        /* create the new array and map to convert rgb floats to unsigned */
        A2Methods_UArray2 rgb_array = pixmap->methods->new(pixmap->width,
                                        pixmap->height, sizeof(float_rgb));
        map(rgb_array, apply_rgbf_to_rgb, pixmap);

        /* free the unused array, set the new array to pixmap's pixels */
        A2Methods_UArray2 to_free = pixmap->pixels;
        pixmap->pixels = rgb_array;
        pixmap->methods->free(&to_free);
        return pixmap;
}

/* apply_rgb_to_rgbf
 *      Purpose: Convert all Pnm_rgbs in a pixmap from unsigned to floats.
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
static void apply_rgb_to_rgbf(int col, int row, A2Methods_UArray2 uarray2,
                               void *elem, void *cl)
{
        Pnm_ppm           local_ppm = cl;
        A2Methods_UArray2 orig      = local_ppm->pixels;

        /* convert denominator from unsigned to float */
        float img_denominator = (float)local_ppm->denominator;

        /* pointer to the pnm_rgb index from closure */
        Pnm_rgb pixel = (A2Methods_Object *)(local_ppm->methods->at(orig, 
                                                                col, row));
        /* create rgb float to be inserted into array, insert it */
        float_rgb temp = singular_rgb_to_rgbf(pixel, img_denominator);
        memcpy(elem, &temp, sizeof(float_rgb));

        (void)uarray2;
}


/* singular_rgb_to_rgbf
 *      Purpose: Converts a singular unsigned rgb pixel to float form
 *   Parameters: pixel: a rgb that is in unsigned ints
 *               img_denominator: image's denominator in float form
 * Expectations: none
 *      Returns: a single float_rgb, which is rgb values in floats
 */
static float_rgb singular_rgb_to_rgbf(Pnm_rgb pixel, float img_denominator)
{
        float_rgb to_return;
        /* descale the r/g/b values using the denominator */
        to_return.r = pixel->red / img_denominator;
        to_return.g = pixel->green / img_denominator;
        to_return.b = pixel->blue / img_denominator;

        return to_return;
}


/* apply_rgb_to_rgbf
 *      Purpose: Convert all Pnm_rgbs in a pixmap from component
 *               video structs to floats
 *   Parameters: col, row: coordinates of the pixel
 *               uarray2: unused
 *               elem: the spot in the uarray2 that we are currently at
 *               cl: pointer to a Pnm_ppm that holds float rgb array
 * Expectations: orig is not NULL
 *               0 <= col < width of orig
 *               0 <= row < height of orig
 *               closure is not NULL 
 *      Returns: none
 */
static void apply_rgbf_to_cv(int col, int row, A2Methods_UArray2 uarray2,
                               void *elem, void *cl)
{
        Pnm_ppm           local_ppm = cl;
        A2Methods_UArray2 orig      = local_ppm->pixels;

        /* pointer to the rgb float index from closure */
        float_rgb *pixel = (A2Methods_Object *)(local_ppm->methods->at(orig,
                                                                col, row));
        /* get the rgb floats in local variables */
        float r = pixel->r;
        float g = pixel->g;
        float b = pixel->b;

         /* create and initialize the struct to be inserted, also performing
            calculations to get them into y, pb, pr values */
        Component_Video to_return;
        to_return.y = (0.299 * r) + (0.587 * g) + (0.114 * b);
        to_return.pb = (-0.168736 * r) - (0.331264 * g) + (0.5 * b);
        to_return.pr = (0.5 * r) - (0.418688 * g) - (0.081312 * b);

        /* copy the new struct into the uarray2 */
        memcpy(elem, &to_return, sizeof(Component_Video));

        (void)uarray2;
}


/* apply_cv_to_rgbf
 *      Purpose: Convert all Pnm_rgbs in a pixmap from component video to
 *               rgb floats.
 *   Parameters: col, row: coordinates of the pixel
 *               uarray2: unused
 *               elem: the spot in the uarray2 that we are currently at
 *               cl: pointer to a Pnm_ppm that component video structs array
 * Expectations: orig is not NULL
 *               0 <= col < width of orig
 *               0 <= row < height of orig
 *               closure is not NULL 
 *      Returns: none
 */
static void apply_cv_to_rgbf(int col, int row, A2Methods_UArray2 uarray2,
                                void *elem, void *cl)
{
        Pnm_ppm           local_ppm = cl;
        A2Methods_UArray2 orig      = local_ppm->pixels;

        /* pointer to the component video index from closure */
        Component_Video *cv = (A2Methods_Object *)(local_ppm->methods->at(orig,
                                                                col, row));
        /* create and initialize the struct to be inserted, clamping the
           values into acceptable values between 0 and 1 */
        float_rgb to_insert;
        to_insert.r = clamp((cv->y + 1.402 * cv->pr), 0, 1);
        to_insert.g = clamp((cv->y - (0.344136 * cv->pb) - (0.714136 *cv->pr)),
                                                                        0 , 1);
        to_insert.b = clamp((cv->y + (1.772 * cv->pb)), 0, 1);

        /* copy the new struct into the uarray2 */
        memcpy(elem, &to_insert, sizeof(float_rgb));

        (void)uarray2;
}


/* apply_rgbf_to_rgb
 *      Purpose: Convert all rgb_floats in a pixmap from floats to unsigned.
 *   Parameters: col, row: coordinates of the pixel
 *               uarray2: unused
 *               elem: the spot in the uarray2 that we are currently at
 *               cl: pointer to a Pnm_ppm that holds rgb_floats array
 * Expectations: orig is not NULL
 *               0 <= col < width of orig
 *               0 <= row < height of orig
 *               closure is not NULL 
 *      Returns: none
 */
static void apply_rgbf_to_rgb(int col, int row, A2Methods_UArray2 uarray2,
                                void *elem, void *cl)
{
        Pnm_ppm           local_ppm = cl;
        A2Methods_UArray2 orig      = local_ppm->pixels;
        
        /* pointer to the rgb float index from closure */
        float_rgb *rgb_vals;
        rgb_vals = (float_rgb *)(local_ppm->methods->at(
            orig, col, row));

        /* create and initialize the struct to be inserted */
        struct Pnm_rgb to_insert = {
                .red = rgb_vals->r * DENOMINATOR,
                .green = rgb_vals->g * DENOMINATOR,
                .blue = rgb_vals->b * DENOMINATOR
        };

        /* copy the new struct into the uarray2 */
        memcpy(elem, &to_insert, local_ppm->methods->size(uarray2));

        (void) uarray2;
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