/**************************************************************
 *
 *                     ppmdiff.c
 *
 *     Assignment: CS40 HW4 arith
 *     Authors:  shakka01, cbolin01
 *     Date:     02/24/23
 *
 *     Implementation of ppmdiff, a program that takes in two images
 *     and determines the difference between them in a percentage value.
 *
 **************************************************************/
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "assert.h"
#include "pnm.h"
#include "a2methods.h"
#include "a2plain.h"
#include "a2blocked.h"

#define header_fmt "COMP40 Compressed image format 2\n%u %u"

typedef struct images {
        int      smaller_width;    /* the smaller width of the two images */
        int      smaller_height;   /* the smaller height of the two images */
        unsigned int pixel_difference_sum; /* sum of differences in all r,g,b
                                        pixels of the images */
        int denominator; /* the denominator scalar for pixels */
        
        Pnm_ppm image;       /* the original image pixmap */
        Pnm_ppm image_prime; /* the second image pixmap */
} *IS;

int calc_smaller_dim(int dim1, int dim2);
void sum_intensities(int col, int row, A2Methods_UArray2 first_image, 
                     void *elem, void *closure);


int main(int argc, char *argv[]) {
        FILE *first_fp;
        FILE *second_fp;
        IS images = malloc(sizeof(*images));
        double E;

        A2Methods_T methods = uarray2_methods_plain;
        assert(methods);
        A2Methods_mapfun *map = methods->map_row_major;
        assert(map);

        assert(argc == 3);
        
        if (strcmp(argv[1], "-") == 0) {
            first_fp = stdin;
        } else {
            first_fp = fopen(argv[1], "r");
            assert(first_fp != NULL);
        }
        
        if (strcmp(argv[2], "-") == 0 && first_fp != stdin) {
            second_fp = stdin;
        } else {
            second_fp = fopen(argv[2], "r");
            assert(second_fp != NULL);
        }

        /* set the fields of the images struct */
        images->image = Pnm_ppmread(first_fp, methods);
        images->image_prime = Pnm_ppmread(second_fp, methods);
        images->smaller_width = 
                calc_smaller_dim(images->image->width,
                                 images->image_prime->width);
        images->smaller_height = 
                calc_smaller_dim(images->image->height,
                                 images->image_prime->height);
        images->denominator = images->image->denominator;
        images->pixel_difference_sum = 0;

        /* sum the pixel intensities */
        map(images->image->pixels, sum_intensities, images);


        E = sqrt(1.0 * images->pixel_difference_sum / (1.0 * 3 *      
                                                      (images->smaller_width) *
                                                     (images->smaller_height)));
        E /= images->denominator;

        printf("%.4f\n", E);


        /* free everything */
        Pnm_ppmfree(&images->image);
        Pnm_ppmfree(&images->image_prime);
        free(images);
        fclose(first_fp);
        fclose(second_fp);
        exit(EXIT_SUCCESS);
}


/* sum_intensities
 *      Purpose: Apply function that calculates the difference in intensities
 *               of each pixel of two different images
 *   Parameters: rotation: the degree to rotate the image by (0, 90, 180, or
 *                         270)
 *               our_ppm: a pointer to a Pnm_ppm struct representing the image
 *               map: a pointer to a map function for the A2Methods_T interface
 *               methods: an A2Methods_T interface
 *               original_pixels: a UArray2 holding the original pixel data
 * Expectations: NULL our_ppm or methods, incorrect or negative dimensions
 *      Returns: nothing
 */
void sum_intensities(int col, int row, A2Methods_UArray2 first_image, 
                     void *elem, void *closure) {
        
        IS local_imgs = closure;

        if (col > local_imgs->smaller_width - 1 || 
            row > local_imgs->smaller_height - 1) {
                return;
        }

        Pnm_ppm image_2 = local_imgs->image_prime;
        Pnm_rgb pixel_1 = (A2Methods_Object *)
                                  (local_imgs->image->methods->at(first_image, col, row));
        Pnm_rgb pixel_2 = (A2Methods_Object *)
                                 (image_2->methods->at(image_2->pixels,
                                  col, row));
        
        int red_diff = pixel_1->red - pixel_2->red;
        int green_diff = pixel_1->green - pixel_2->green;
        int blue_diff = pixel_1->blue - pixel_2->blue;

        int diff_sum = (red_diff * red_diff) + (green_diff * green_diff) +
                          (blue_diff * blue_diff);

        local_imgs->pixel_difference_sum += diff_sum;
        
        (void)elem;
}

/* calc_smaller_dim
 *      Purpose: Calculates the smaller dimension and raises CRE if they
 *               differ by more than 1
 *   Parameters: dim1, dim2: the width or height dimensions of images
 * Expectations: dimensions differ by no more than 1
 *      Returns: the smaller dimension
 */
int calc_smaller_dim(int dim1, int dim2) 
{
        int difference = dim1 - dim2;
        if (difference > 1 || difference < -1) {
                fprintf(stderr, "Dimensions differ by more than 1\n");
                exit(EXIT_FAILURE);
        }
        if (dim1 < dim2) {
                return dim1;
        }
        else {
                return dim2;
        }
}