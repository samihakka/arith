/**************************************************************
 *
 *                     uarray2b.c
 *
 *     Assignment: CS40 HW3 iii
 *     Authors:  shakka01, cbolin01
 *     Date:     02/19/23
 *
 *     Implementation of UArray2b, a blocked 2-D unboxed array that holds
 *     the type of data that the client specifies. Contains definitions
 *     of the functions in the UArray2b. A blocked array stores cells in
 *     each block that are located next to each other in memory, and this
 *     implementation supports block-major mapping, visiting all cells in
 *     each block before moving to the next.
 *
 **************************************************************/

#include "uarray2b.h"
#include "uarray.h"
#include "uarray2.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>


#define T UArray2b_T

/* stores largest possible element size for new_64K_block */
const int SIXTY_FOUR_KB = 64 * 1024;

struct T {
        int       width;      /* number of cells in UArray2b width-wise */
        int       height;     /* number of cells in UArray2b height-wise */
        int       size;       /* element size--num bytes per element */
        int       blocksize;  /* number of cells on one side of a block */
        UArray2_T container;  /* UArray2 that holds pointers to the UArrays
                                 that make up UArray2b blocks */
        int container_width;  /* number of blocks in UArray2b width-wise */
        int container_height; /* number of blocks in UArray2b height-wise */
};

int container_dim(int dim, int blocksize);

/* UArray2b_new
 *     Purpose: Create and allocate space for a UArray2b array on the heap.
 *  Parameters: height: the number of rows
 *              width:  the length of each row (or num columns)
 *              size:   the amount of space each element consumes
 *              blocksize: number of cells on each side of a block;
 *                         number of cells/block = blocksize^2
 *     Expects: valid width, height, size, and blocksize (all > 0)
 *     Returns: A newly allocated UArray2b
 */
T UArray2b_new(int width, int height, int size, int blocksize) {
        assert(width > 0 && height > 0 && size > 0 && blocksize > 0);
        T uarray2b = malloc(sizeof(*uarray2b));
        assert(uarray2b != NULL);

        /* set members of the uarray2b struct */
        uarray2b->width            = width;
        uarray2b->height           = height;
        uarray2b->size             = size;
        uarray2b->blocksize        = blocksize;
        uarray2b->container_width  = container_dim(width, blocksize);
        uarray2b->container_height = container_dim(height, blocksize);
        uarray2b->container =
            UArray2_new(uarray2b->container_width, uarray2b->container_height,
                        sizeof(UArray2_T));
        assert(uarray2b->container != NULL);

        /* determine & store num cells to be in each single uarray */
        int num_cells = blocksize * blocksize;

        /* initialize the container array with UArrays */
        for (int i = 0;
             i < uarray2b->container_width * uarray2b->container_height; i++) {
                /* store ptr to ith index of container array */
                UArray_T *blockp = UArray2_at(uarray2b->container,
                                              i % uarray2b->container_width,
                                              i / uarray2b->container_width);
                /* populate the index with a fresh array with blocksize *
                   blocksize cells */
                *blockp = UArray_new(num_cells, size);
                assert(blockp != NULL);
        }

        return uarray2b;
}


/* UArray2b_new_64K_block
 *      Purpose: Create new UArray2b with a default blocksize that is large
                 as possible while still fitting a block within 64KB
 *   Parameters: width: number of cells in UArray2b width-wise
                 height: number of cells in UArray2b height-wise
                 size: element size--num bytes per element
 * Expectations: valid width, height, and size (all > 0)
 *      Returns: a newly created UArray2b
 */
T UArray2b_new_64K_block(int width, int height, int size) {
        assert(width > 0 && height > 0 && size > 0);

        /* if size is greater than max elt size, default blocksize to 1 */
        int blocksize;
        if (size > SIXTY_FOUR_KB) {
                blocksize = 1;
        } else { /* find blocksize by getting the root of max elt size / size
                    specified. Have to get floor so we round down */
                blocksize = (int)floor(sqrt(SIXTY_FOUR_KB / size));
        }
        /* use previously defined new function once we have found blocksize */
        return UArray2b_new(width, height, size, blocksize);
}

/* UArray2b_free
 *     Purpose: Free the specified UArray2b's allocated heap memory
 *  Parameters: Pointer to a UArray2b
 *     Expects: A valid UArray2b is passed in. If it is invalid,
 *              a CRE will be raised
 *     Effects: frees associated UArray2b heap memory
 */
void UArray2b_free(T *uarray2b) {
        assert(*uarray2b != NULL);
        int first_param;
        int second_param;

        /* Free all of the arrays stored in the container */
        for (int i = 0;
             i < (*uarray2b)->container_width * (*uarray2b)->container_height;
             i++) {
                first_param  = i % (*uarray2b)->container_width;
                second_param = i / (*uarray2b)->container_width;
                /* store ptr to ith index of container array */
                UArray_T *blockp = UArray2_at((*uarray2b)->container,
                                              first_param, second_param);
                /* free the UArray at the current block */
                UArray_free(&(*blockp));
                blockp = NULL;
        }

        /* free the container and then the uarray2b struct */
        UArray2_free(&(*uarray2b)->container);
        free(*uarray2b);
}

/* UArray2b_width
 *      Purpose: Provide client with the width of the UArray2b
 *   Parameters: A UArray2b instance
 * Expectations: A valid UArray2b is passed in. If it is invalid,
 *               it will raise a CRE
 *      Returns: the width, or how many cells wide the UArray2b is
 */
int UArray2b_width(T uarray2b) {
        assert(uarray2b != NULL);
        return uarray2b->width;
}

/* UArray2b_height
 *      Purpose: Provide client with the height of the UArray2b
 *   Parameters: A UArray2b instance
 * Expectations: A valid UArray2b is passed in. If it is invalid,
 *               we will raise a CRE
 *      Returns: the width, or how many cells tall the UArray2b is
 */
int UArray2b_height(T uarray2b) {
        assert(uarray2b != NULL);
        return uarray2b->height;
}


/* UArray2b_size
 *      Purpose: Allow client to access the UArray2b's element size
 *   Parameters: A UArray2b instance
 * Expectations: A valid UArray2b is passed in. If it is invalid,
 *               we will raise a CRE
 *      Returns: the size of the elements that the UArray2b contains
 */
int UArray2b_size(T uarray2b) {
        assert(uarray2b != NULL);
        return uarray2b->size;
}

/* UArray2b_blocksize
 *      Purpose: Provide client with the blocksize of the UArray2b
 *   Parameters: A UArray2b instance
 * Expectations: A valid UArray2b is passed in. If it is invalid,
 *               the UArray2_blocksize function that is being called
 *               will raise a CRE
 *      Returns: the blocksize in the desired UArray2b
 */
int UArray2b_blocksize(T uarray2b) {
        assert(uarray2b != NULL);
        return uarray2b->blocksize;
}

/* UArray2b_at
 *      Purpose: Access an element at a certain index in the UArray2
 *   Parameters: A UArray2
 *             row: the y coordinate of the desired element
 *             col: the x coordinate of the desired element
 * Expectations: NULL array, incorrect or negative dimensions
 *      Returns: pointer to the element at the desired index
 */
void *UArray2b_at(T uarray2b, int col, int row) {
        assert(uarray2b != NULL);
        assert(col < uarray2b->width && col >= 0);
        assert(row < uarray2b->height && row >= 0);

        /* calculations used to determine correct index to access */
        int blocksize  = UArray2b_blocksize(uarray2b);
        int block_col  = col / blocksize;
        int block_row  = row / blocksize;
        int cell_index = (col % blocksize) * blocksize + row % blocksize;

        /* store the pointer to the block that we need to access */
        UArray_T *blockp =
            UArray2_at(uarray2b->container, block_col, block_row);
        /* return the element of the uarray at that block at specified index */
        return UArray_at(*blockp, cell_index);
}

/* UArray2b_map
 *     Purpose: Calls the parameterized apply function to every element in the
                UArray2b in block major order (visits entire block, then moves
                to the next block)
 *  Parameters: T: A UArray2b
                apply_function: any function that the client wants to pass in,
                                will be applied to every cell in the UArray2b
                        -T: a UArray2b
                        -col, row: values that determine the index to access
                        -elem: void pointer to the element at current index
                        -closure: void pointer to whatever the client desires
                closure: void pointer to whatever the client desires
 * Expectations: A valid UArray2b. If it is NULL, CRE will be raised
 *     Returns: None
 */
void UArray2b_map(T     uarray2b,
                  void  apply(int col, int row, T uarray2b, void *elem,
                              void *closure),
                  void *closure) {
        assert(uarray2b != NULL);
        int block_index_w, block_index_h, cell_x, cell_y;
        int bs = uarray2b->blocksize;

        for (int i = 0;
             i < uarray2b->container_width * uarray2b->container_height; i++) {
                block_index_w    = i % uarray2b->container_width;
                block_index_h    = i / uarray2b->container_width;
                UArray_T *blockp = UArray2_at(uarray2b->container,
                                              block_index_w, block_index_h);

                int len = UArray_length(*blockp);

                cell_x = block_index_w * bs;
                cell_y = block_index_h * bs;

                for (int cell_index = 0; cell_index < len; cell_index++) {
                        int x = cell_x + cell_index / bs;
                        int y = cell_y + cell_index % bs;
                        if (x < uarray2b->width &&
                            y < uarray2b->height) {
                                apply(x, y, uarray2b,
                                      UArray_at(*blockp, cell_index), closure);
                        }
                }
                //UArray_free(blockp);
        }
}


/* PRIVATE HELPER FUNCTIONS */

int container_dim(int dim, int blocksize) {
        int return_dimension = dim / blocksize;
        if ((dim % blocksize) != 0) { return_dimension++; }
        return return_dimension;
}