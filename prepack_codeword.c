/**************************************************************
 *
 *                     prepack_codeword.c
 *
 *     Assignment: CS40 HW4 arith
 *     Authors:  shakka01, cbolin01
 *     Date:     02/24/23
 *
 *     Implementation of prepack_codeword, allowing us to convert from
 *     prepack structs to codewords and back again.
 *
 **************************************************************/

#include "prepack_codeword.h"
#include "bitpack.h"

/* contains all of the information needed to convert to/from codewords */
typedef struct PrePack {
        uint64_t a;
        int64_t b;
        int64_t c;
        int64_t d;
        unsigned index_pb;
        unsigned index_pr;
} PrePack;

static void apply_pack_bits(int col, int row, A2Methods_UArray2 uarray2,
                            void *elem, void *cl);
static uint32_t singular_bitpack(PrePack *pp);

static void apply_unpack_bits(int col, int row, A2Methods_UArray2 uarray2,
                            void *elem, void *cl);
static PrePack singular_bitunpack(uint32_t *cw_p);


/* pack_bits
 *      Purpose: Takes in a Pnm_ppm of PrePack structs and calls a map
 *               function that packs these structs into 4 byte 
 *               codewords
 *   Parameters: Pnm_ppm struct containing PrePack's
 * Expectations: none
 *      Returns: a single float_rgb, which is rgb values in floats
 */
Pnm_ppm pack_bits(Pnm_ppm prepack_map)
{
        assert(prepack_map != NULL);
        A2Methods_mapfun *map = prepack_map->methods->map_row_major;

        /* create the new array and map to convert prepacks to codewords */
        unsigned width = prepack_map->methods->width(prepack_map->pixels);
        unsigned height = prepack_map->methods->height(prepack_map->pixels);
        A2Methods_UArray2 codeword_array = prepack_map->methods->new(width, \
                                    height, sizeof(uint32_t));
        map(codeword_array, apply_pack_bits, prepack_map);

        /* free the unused array, set the new array to pixmap's pixels */
        A2Methods_UArray2 to_free = prepack_map->pixels;
        prepack_map->pixels = codeword_array;
        prepack_map->methods->free(&to_free);
        return prepack_map;
}


/* apply_pack_bits
 *      Purpose: Map over the PrePack uarray2 and call a helper
                 function that converts this into a 4 byte
                 codeword, store the codeword in the new array
 *   Parameters: col, row: coordinates of the codeword
 *               uarray2: unused
 *               elem: the spot in the uarray2 that we are currently at
 *               cl: pointer to a Pnm_ppm that holds PrePack's
 * Expectations: none
 *      Returns: none
 */
static void apply_pack_bits(int col, int row, A2Methods_UArray2 uarray2,
                            void *elem, void *cl)
{
        Pnm_ppm local_ppm           = cl;
        A2Methods_UArray2 orig      = local_ppm->pixels;

        /* pointer to the index that holds prepack struct */
        PrePack *pp = (A2Methods_Object *)(local_ppm->methods->at(orig, 
                                            col, row));

        /* packs bits into a singular codeword */
        uint32_t to_insert = singular_bitpack(pp);

        /* codeword uarray2 is updated here */
        memcpy(elem, &to_insert, sizeof(uint32_t));
        (void) uarray2;
}


/* singular_bitpack
 *      Purpose: Call bitpacking functions to fit the 6 elements 
 *               contained in the PrePack struct into a 4 byte codeword
 *   Parameters: Pointer to a PrePack struct
 * Expectations: pp is not NULL
 *      Returns: a fully packed uint32_t codeword
 */
static uint32_t singular_bitpack(PrePack *pp)
{
        /* sets the codeword to  0, then we populate the codeword 
           from the prepack struct */
        uint32_t the_codeword = 0;

        the_codeword = Bitpack_newu(the_codeword, 6, 26, pp->a);
        the_codeword = Bitpack_news(the_codeword, 6, 20, pp->b);
        the_codeword = Bitpack_news(the_codeword, 6, 14, pp->c);
        the_codeword = Bitpack_news(the_codeword, 6, 8, pp->d);
        the_codeword = Bitpack_newu(the_codeword, 4, 4,  pp->index_pb);
        the_codeword = Bitpack_newu(the_codeword, 4, 0, pp->index_pr);

        return the_codeword;
}


/* unpack_bits
 *      Purpose: Takes in a Pnm_ppm of bitpacked uint32's
 *       and calls a map function that unpacks these codewords
 *       into PrePack structs
 *      Parameters: Pnm_ppm struct containing codewords
 *      Expectations: bitpacked_map is not NULL
 *      Returns: a Pnm_ppm containing PrePack structs
 */
Pnm_ppm unpack_bits(Pnm_ppm bitpacked_map)
{
        assert(bitpacked_map != NULL);

        A2Methods_mapfun *map = bitpacked_map->methods->map_row_major;

        /* create the new array and map to convert codewords to prepacks */
        unsigned width = bitpacked_map->methods->width(bitpacked_map->pixels);
        unsigned height = bitpacked_map->methods->height(bitpacked_map->pixels);
        A2Methods_UArray2 prepack_array = bitpacked_map->methods->new(width, \
                                    height, sizeof(PrePack));
        map(prepack_array, apply_unpack_bits, bitpacked_map);

        /* free the unused array, set the new array to pixmap's pixels */
        A2Methods_UArray2 to_free = bitpacked_map->pixels;
        bitpacked_map->pixels = prepack_array;
        bitpacked_map->methods->free(&to_free);
        return bitpacked_map;
}


/* apply_unpack_bits
 *      Purpose: Map over the codeword uarray2 and call a helper
 *               function that converts this into a PrePack struct,
 *               store this value in the new array
 *   Parameters: col, row: coordinates of the codeword
 *               uarray2: unused
 *               elem: the spot in the uarray2 that we are currently at
 *               cl: pointer to a Pnm_ppm that holds codewords
 * Expectations: none
 *      Returns: none
 */
static void apply_unpack_bits(int col, int row, A2Methods_UArray2 uarray2,
                            void *elem, void *cl)
{
        Pnm_ppm local_ppm           = cl;
        A2Methods_UArray2 orig      = local_ppm->pixels;

        /* pointer to the codeword at this index */
        uint32_t *cw_p = (A2Methods_Object *)(local_ppm->methods->at(orig, 
                                            col, row));

        /* unpacks bits from codeword to a prepack struct and inserts it */
        PrePack to_insert = singular_bitunpack(cw_p);
        memcpy(elem, &to_insert, sizeof(PrePack));

        (void) uarray2;
}


/* singular_bitunpack
 *      Purpose: Call bitpacking functions (in this case, unpacking) to
 *               convert the 4 byte codeword into 6 separate values
 *   Parameters: Pointer to a codeword
 * Expectations: cw_p is not NULL
 *      Returns: an unpacked "PrePack" struct
 */
static PrePack singular_bitunpack(uint32_t *cw_p)
{
        PrePack to_return;

        /* get signed and unsigned from the codeword and set
           the fields of prepack struct */
        to_return.a = Bitpack_getu(*cw_p, 6, 26);
        to_return.b = Bitpack_gets(*cw_p, 6, 20);
        to_return.c = Bitpack_gets(*cw_p, 6, 14);
        to_return.d = Bitpack_gets(*cw_p, 6, 8);
        to_return.index_pb = Bitpack_getu(*cw_p, 4, 4);
        to_return.index_pr = Bitpack_getu(*cw_p, 4, 0);

        return to_return;
}