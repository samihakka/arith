/**************************************************************
 *
 *                     uarray2.c
 *
 *     Assignment: CS40 HW3 Locality
 *     Authors:  shakka01, cbolin01
 *     Date:     02/21/23
 *
 *     Implementation of UArray2, a 2-D unboxed array that holds
 *     the type of data that the client specifies. Contains definitions
 *     of the functions in the UArray2.
 *
 **************************************************************/

#include "uarray.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <uarray2.h>

#define T UArray2_T

struct T {
        int      height;    /* number of rows */
        int      width;     /* number of columns */
        int      size;      /* element size--num bytes per element */
        UArray_T container; /* uarray that holds pointers to the uarrays
                               that make up the UArray2 */
};

/* UArray2_new
 *     Purpose: Create and allocate space for a new 2-D array on the heap.
 *  Parameters: height: the number of rows
 *             width:  the length of each row (or num columns)
 *             size:   the amount of space each element consumes
 * Error Cases: bad malloc, incorrect dimensions
 *     Returns: Pointer to the newly allocated UArray2
 * 
 */
T UArray2_new(int width, int height, int size) {
        /* declare and create the new uarray2 */
        T my_array = malloc(sizeof(*my_array));
        assert(my_array != NULL);

        /* initialize new array's fields */
        my_array->height = height;
        my_array->width  = width;
        my_array->size   = size;
        /* container has height indices to store the rows */
        my_array->container = UArray_new(height, sizeof(UArray_T));
        assert(UArray_length(my_array->container) == height);

        /* initialize the container array with UArrays */
        for (int i = 0; i < height; i++) {
                /* store ptr to ith index of container array */
                UArray_T *rowp = UArray_at(my_array->container, i);
                /* populate the index with a fresh array of size width */
                *rowp = UArray_new(width, size);
                assert(UArray_length(*rowp) == width);
        }

        /* return the new UArray2 */
        return my_array;
}

/* UArray2_free
 *     Purpose: Free the specified UArray2's allocated heap memory
 *  Parameters: A UArray2 pointer
 * Error Cases: NULL array
 *     Effects: frees associated UArray heap memory
 */
void UArray2_free(T *my_array) {
        assert(*my_array != NULL);
        /* Free all of the arrays stored in the container */
        for (int i = 0; i < (*my_array)->height; i++) {
                /* store ptr to ith index of container array */
                UArray_T *rowp = UArray_at((*my_array)->container, i);
                /* populate the index with a fresh array of size width */
                UArray_free(&(*rowp));
        }

        /* free the container and then the 2-D array struct */
        UArray_free(&(*my_array)->container);
        free(*my_array);
}

/* UArray2_width
 *     Purpose: Allow client to access the number of columns in the UArray2
 *  Parameters: A UArray2
 * Error Cases: NULL array
 *     Returns: the width, or num cols in the desired UArray2
 */
int UArray2_width(T my_array) {
        assert(my_array != NULL);
        return my_array->width;
}

/* UArray2_height
 *     Purpose: Allow client to access the number of rows in the UArray2
 *  Parameters: A UArray2
 * Error Cases: NULL array
 *     Returns: the height, or num rows in the desired UArray2
 */
int UArray2_height(T my_array) {
        assert(my_array != NULL);
        return my_array->height;
}


/* UArray2_size
 *     Purpose: Allow client to access the UArray2's element size
 *  Parameters: A UArray2
 * Error Cases: NULL array
 *     Returns: the size of the elements that the UArray2 contains
 */
int UArray2_size(T my_array) {
        assert(my_array != NULL);
        return my_array->size;
}

/* UArray2_at
 *     Purpose: Access an element at a certain index in the UArray2
 *  Parameters: A UArray2
 *             row: the y coordinate of the desired element
 *             col: the x coordinate of the desired element
 * Error Cases: NULL array, incorrect or negative dimensions
 *     Returns: pointer to the element at the desired index
 */
void *UArray2_at(T my_array, int col, int row) {
        assert(my_array != NULL);
        assert(col < my_array->width && col >= 0);
        assert(row < my_array->height && row >= 0);

        /* go to the correct row in container, follow its
           pointer, and then access the correct column */
        UArray_T *rowp = UArray_at(my_array->container, row);

        /* return the address at that index */
        return UArray_at(*rowp, col);
}

/* UArray2_map_row_major
 *     Purpose: Calls the parameterized apply function to every element in the
               UArray2 in row major order (visits entire row, then moves to
               the next row)
 *  Parameters: T: A UArray2
               apply_function: any function that the client wants to pass in
                        row, col: specified y and x coordinates
                        T: a UArray2
                        data: void pointer to the element that the index holds
                        closure: void pointer to whatever the client desires
                closure: void pointer to whatever the client desires
 * Error Cases: NULL array
 *     Returns: None
 */
void UArray2_map_row_major(T my_array, UArray2_applyfun apply,
                           void *closure) {
        assert(my_array != NULL);
        UArray_T *rowp = NULL;
        /* loop through the rows (j variable) */
        for (int i = 0; i < my_array->height; i++) {
                /* loop through all the indices of each row */
                for (int j = 0; j < my_array->width; j++) {
                        rowp = UArray_at(my_array->container, i);
                        apply(j, i, my_array, UArray_at(*rowp, j), closure);
                }
        }
}

/* UArray2_map_col_major
 *     Purpose: Calls the parameterized apply function to every element in the
               UArray2 in col major order (visits entire col, then moves to
               the next col)
 *  Parameters: T: A UArray2
               apply_function: any function that the client wants to pass in
                        row, col: specified y and x coordinates
                        T: a UArray2
                        data: void pointer to the element that the index holds
                        closure: void pointer to whatever the client desires
                closure: void pointer to whatever the client desires
 * Error Cases: NULL array
 *     Returns: None
 */
void UArray2_map_col_major(T my_array, UArray2_applyfun apply,
                           void *closure) {
        assert(my_array != NULL);
        UArray_T *rowp = NULL;
        /* loop through the rows (j variable) */
        for (int i = 0; i < my_array->width; i++) {
                /* loop through all the indices of each row */
                for (int j = 0; j < my_array->height; j++) {
                        rowp = UArray_at(my_array->container, j);
                        apply(i, j, my_array, UArray_at(*rowp, i), closure);
                }
        }
}