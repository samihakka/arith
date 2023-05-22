/**************************************************************
 *
 *                     UArray2.h
 *
 *     Assignment: CS40 HW3 Locality
 *     Authors:  shakka01, cbolin01
 *     Date:     02/13/23
 *
 *     Interface for UArray2, a 2-D unboxed array that holds any
 *     type of data that the client specifies. Allows clients to
 *     create new UArray2s, check the height, width, size, get an element
 *     at a specific index, and map functions in column and row major order.
 *
 **************************************************************/

#ifndef UARRAY2_INCLUDED
#define UARRAY2_INCLUDED

#define T UArray2_T

typedef struct T *T;
typedef void UArray2_applyfun(int col, int row, T my_array, void *element,
                                    void *closure);
typedef void UArray2_mapfun(T my_array, UArray2_applyfun apply,
                             void *closure);


extern T     UArray2_new(int width, int height, int size);
extern int   UArray2_height(T my_array);
extern int   UArray2_width(T my_array);
extern int   UArray2_size(T my_array);
extern void *UArray2_at(T my_array, int col, int row);
extern void  UArray2_free(T *my_array);
extern void  UArray2_map_row_major(T     my_array, UArray2_applyfun apply,
                                   void *closure);
extern void  UArray2_map_col_major(T     my_array, UArray2_applyfun apply,
                                   void *closure);

#undef T
#endif