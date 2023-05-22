#include <string.h>

#include <a2plain.h>
#include "uarray2.h"

const int BLOCKSIZE = 1;

/************************************************/
/* Define a private version of each function in */
/* A2Methods_T that we implement.               */
/************************************************/

static A2Methods_UArray2 new(int width, int height, int size)
{
        return UArray2_new(width, height, size);
}

static A2Methods_UArray2 new_with_blocksize(int width, int height, int size,\
                                            int blocksize)
{
        (void) blocksize;
        return UArray2_new(width, height, size);
}

/* ============================================================

                         OUR FUNCTIONS

   ============================================================
*/ 

/* free
 *      Purpose: Free the memory associated with the given instance of a
                 UArray2
 *   Parameters: A UArray2 instance
 * Expectations: A valid uarray2 is passed in. If it is invalid,
 *               the UArray2_size function that is being called 
 *               will raise a CRE
 *      Returns: none
 */
static void a2free(A2Methods_UArray2 * uarray2)
{
        UArray2_free((UArray2_T *)uarray2);
}

/* width
 *      Purpose: Provide client with the width of the UArray2
 *   Parameters: A UArray2 instance
 * Expectations: A valid uarray2 is passed in. If it is invalid,
 *               the UArray2_width function that is being called 
 *               will raise a CRE
 *      Returns: the width, or num cols in the desired UArray2
 */
static int width(A2Methods_UArray2 uarray2)
{
        return UArray2_width(uarray2);
}

/* height
 *      Purpose: Provide client with the height of the UArray2
 *   Parameters: A UArray2 instance
 * Expectations: A valid uarray2 is passed in. If it is invalid,
 *               the UArray2_height function that is being called 
 *               will raise a CRE
 *      Returns: the height, or num rows in the desired UArray2
 */
static int height(A2Methods_UArray2 uarray2) 
{
        return UArray2_height(uarray2);
}

/* size
 *      Purpose: Return the size of the elements being stored in the UArray2
 *   Parameters: A UArray2 instance
 * Expectations: A valid uarray2 is passed in. If it is invalid,
 *               the UArray2_size function that is being called 
 *               will raise a CRE
 *      Returns: the size of an element in the desired UArray2 
 */
static int size(A2Methods_UArray2 uarray2) 
{
        return UArray2_size(uarray2);
}

/* blocksize
 *      Purpose: Essentially, this is a placeholder for the blocksize function.
                 It exists for continuity
 *   Parameters: A UArray2 instance
 * Expectations: A valid uarray2 is passed in. If it is invalid,
 *               the UArray2_size function that is being called 
 *               will raise a CRE
 *      Returns: the blocksize of a plain UArray2, which is 1
 */
static int blocksize(A2Methods_UArray2 uarray2)
{
        (void)uarray2;
        return BLOCKSIZE;
}

/* at
 *      Purpose: Return an A2Methods Object, which is a pointer to an
                 element of the UArray2 passed into the function
 *   Parameters: A UArray2 instance
                 Column of the desired element
                 Row of the desired element
 * Expectations: A valid uarray2 is passed in. If it is invalid,
 *               the UArray2_size function that is being called 
 *               will raise a CRE. Row and column are nonnegative and
 *               within the bounds of the UArray2. These cases result in CRE.
 *      Returns: the element at the desired index of the UArray2 
 */
static A2Methods_Object *at(A2Methods_UArray2 uarray2, int col, int row) 
{
        return UArray2_at(uarray2, col, row);
}


/* ============================================================

                        GIVEN FUNCTIONS

   ============================================================
*/ 

static void map_row_major(A2Methods_UArray2 uarray2,
                          A2Methods_applyfun apply,
                          void *cl)
{
        UArray2_map_row_major(uarray2, (UArray2_applyfun*)apply, cl);
}

static void map_col_major(A2Methods_UArray2 uarray2,
                          A2Methods_applyfun apply,
                          void *cl)
{
        UArray2_map_col_major(uarray2, (UArray2_applyfun*)apply, cl);
}

struct small_closure {
        A2Methods_smallapplyfun *apply; 
        void                    *cl;
};

static void apply_small(int i, int j, UArray2_T uarray2,
                        void *elem, void *vcl)
{
        struct small_closure *cl = vcl;
        (void)i;
        (void)j;
        (void)uarray2;
        cl->apply(elem, cl->cl);
}

static void small_map_row_major(A2Methods_UArray2        a2,
                                A2Methods_smallapplyfun  apply,
                                void *cl)
{
        struct small_closure mycl = { apply, cl };
        UArray2_map_row_major(a2, apply_small, &mycl);
}

static void small_map_col_major(A2Methods_UArray2        a2,
                                A2Methods_smallapplyfun  apply,
                                void *cl)
{
        struct small_closure mycl = { apply, cl };
        UArray2_map_col_major(a2, apply_small, &mycl);
}

/*
 * A2Methods 
 */
static struct A2Methods_T uarray2_methods_plain_struct = {
        new,
        new_with_blocksize,
        a2free,
        width,
        height,
        size,
        blocksize,
        at,
        map_row_major,
        map_col_major,
        NULL,                     // map_block_major
        map_row_major,            // map_default
        small_map_row_major,
        small_map_col_major,
        NULL,                     // small_map_block_major
        small_map_row_major,      // small_map_default
        
};

A2Methods_T uarray2_methods_plain = &uarray2_methods_plain_struct;
