/**************************************************************
 *
 *                     bitpack.c
 *
 *     Assignment: CS40 HW4 arith
 *     Authors:  shakka01, cbolin01
 *     Date:     02/24/23
 *
 *     Implementation of bitpack, which contains functions to manipulate
 *     64 bit integers, check to see if an int can fit in certain number of
 *     bits, and replace fields of integers with new bits.
 *
 **************************************************************/
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <string.h>
#include "except.h"
#include "assert.h"
#include "bitpack.h"

uint64_t get_exponent(unsigned base, unsigned exponent);
uint64_t make_mask(unsigned width, unsigned lsb);

Except_T Bitpack_Overflow = { "Overflow packing bits" };

const unsigned int MAX_SHIFT = 64;


/* Bitpack_fitsu
 *      Purpose: Determine if an unsigned int can be represented in the
 *               specified number of bits
 *   Parameters: n: the integer to be tested
 *               width: the number of bits that n can fit/not fit in
 * Expectations: 
 *      Returns: true or false, depending on wether n can be represented in 
 *               "width" bits
 */
bool Bitpack_fitsu(uint64_t n, unsigned width)
{
    assert(width > 0 && width <= MAX_SHIFT);
    /* gets 2^width to determine how many values we can fit in width */
    unsigned upper_bound = get_exponent(2, width);
    if (n >= upper_bound) {
        return false;
    } else {
        return true;
    }
}


/* Bitpack_fitss
 *      Purpose: Determine if a signed int can be represented in the
 *               specified number of bits
 *   Parameters: n: the integer to be tested
 *               width: the number of bits that n can fit/not fit in
 * Expectations: 
 *      Returns: true or false, depending on wether n can be represented in 
 *               "width" bits
 */
bool Bitpack_fitss(int64_t n, unsigned width)
{
    assert(width > 0 && width <= MAX_SHIFT);
    /* gets 2^width to determine how many values we can fit in width */
    unsigned result = get_exponent(2, width);

    /* compute negative and positive bounds because testing signed int */
    int negative_bound = -1 * ((int)result) / 2;
    int positive_bound = ((int)result) / 2 - 1;

    /* only return true if it is within bounds */
    if (n >= negative_bound && n <= positive_bound) {
        return true;
    } else {
        return false;
    }
}


/* Bitpack_getu
 *      Purpose: Returns an unsigned int when given a word, width, and least
 *               significant bit.
 *   Parameters: word: a word represented in 64 or less bits
 *               width: the number of bits that the word should be
 * Expectations: that width is greater than 0 and lsb is greater than 0.
 *               also that lsb + width is less than max shift of 64 bits
 *      Returns: new unsigned word created given the values passed into the 
 *               function
 */
uint64_t Bitpack_getu(uint64_t word, unsigned width, unsigned lsb)
{
    assert(lsb + width <= MAX_SHIFT);
    
    /* just return 0 if width is specified as 0 */
    if (width == 0) {
        return 0;
    }

    uint64_t mask = make_mask(width, lsb);
    uint64_t isolated_field = word & mask; /* now have the isolated field */

    isolated_field >>= lsb; /* isolated field is now accurately represented */

    return isolated_field;
}


/* Bitpack_gets
 *      Purpose: Returns a signed int when given a word, width, and least
 *               significant bit.
 *   Parameters: word: a word represented in 64 or less bits
 *               width: the number of bits that the word should be or fit in
 *               lsb: the least significant bit of the word
 * Expectations: lsb is greater than 0.
 *               also that lsb + width is less than max shift of 64 bits.
 *      Returns: new signed word created given the values passed into the 
 *               function
 */
int64_t Bitpack_gets(uint64_t word, unsigned width, unsigned lsb)
{
    assert(lsb + width <= MAX_SHIFT);
    
    /* just return 0 if width is specified as 0 */
    if (width == 0){
        return 0;   
    }

    /* first get the unsigned representation */
    uint64_t unsigned_field = Bitpack_getu(word, width, lsb);

    /* create a mask for the most significant bit */
    uint64_t mask = make_mask(1, (width - 1));
    uint64_t msb = unsigned_field & mask;

    int64_t signed_field;
    if (msb != 0) {
        /* MSB is 1, so the bits must change for the signed representation */
        //signed_field = (int64_t)unsigned_field;
        signed_field = (unsigned_field - get_exponent(2, width));
    } else {
        signed_field = (int64_t)unsigned_field;
    }
    
    return signed_field;
}

/* Bitpack_newu
 *      Purpose: Create a new word by replacing the bits of given width
 *               with bits passed into the value parameter.
 *   Parameters: word: original word represented in 64 or less bits
 *               width: the number of bits that the value to be inserted
 *               should fit in
 *               lsb: where in the word that value should be placed
 *               
 * Expectations: least significant bit is not negative
 *               width + lsb less than or equal to max value that can be held
 *               also that lsb + width is less than max shift of 64 bits.
 *               Value should be able to fit in the "width" amount of bits.
 *      Returns: new unsigned word created given the values passed into the 
 *               function
 */
uint64_t Bitpack_newu(uint64_t word, unsigned width, unsigned lsb, 
                      uint64_t value)
{
    assert(lsb + width <= MAX_SHIFT);

    /* if it doesn't fit, raise overflow exception */
    if(!Bitpack_fitsu(value, width)) {
            RAISE(Bitpack_Overflow);
    }

    /* just return 0 if width is specified as 0 */
    if (width == 0) {
        return word;
    }

    uint64_t mask = make_mask(width, lsb);
    mask = ~mask;

    /* performing the AND operator on "word" will clear the desired field */
    word = word & mask;
    value <<= lsb;
    /* OR operator will keep transfer "value" into the empty space within
       "word" */
    word = word | value;
    return word;
}


/* Bitpack_news
 *      Purpose: Create a new word by replacing the bits of given width
 *               with bits passed into the value parameter.
 *   Parameters: word: original word represented in 64 or less bits
 *               width: the number of bits that the value to be inserted
 *               should fit in
 *               lsb: the rightmost bit of value passed in
 *               value: an unsigned, 64 bit integer that is the value
 *               to be inserted into word
 * Expectations: that width is greater than 0 and lsb is greater than 0.
 *               also that lsb + width is less than max shift of 64 bits.
 *               Value should be able to fit in the "width" amount of bits.
 *      Returns: new unsigned signed word created given the values passed 
 *               into the function
 */
uint64_t Bitpack_news(uint64_t word, unsigned width, unsigned lsb,  
                      int64_t value)
{
    assert(lsb + width <= MAX_SHIFT);

    /* if it doesn't fit, raise overflow exception */
    if(!Bitpack_fitss(value, width)) {
            RAISE(Bitpack_Overflow);
    }

    /* just return 0 if width is specified as 0 */
    if (width == 0) {
        return word;
    }

    int64_t mask = make_mask(1, (width - 1));

    int64_t msb = value & mask;
    uint64_t unsigned_value;

    if (msb != 0) {
        unsigned_value = get_exponent(2, width) + value;
    } else {
        unsigned_value = value;
    }
    return Bitpack_newu(word, width, lsb, unsigned_value);
}


/* Private helper functions  */

/* get_exponent
 *      Purpose: Returns a signed int when given a word, width, and least
 *               significant bit.
 *   Parameters: word: a word represented in 64 or less bits
 *               width: the number of bits that the word should be or fit in
 * Expectations: that width is greater than 0 and lsb is greater than 0.
 *               also that lsb + width is less than max shift of 64 bits.
 *      Returns: new signed word created given the values passed into the 
 *               function
 */
uint64_t get_exponent(unsigned base, unsigned exponent)
{
    uint64_t result = 1;
    for (;;)
    {
        if (exponent & 1)
            result *= base;
        exponent >>= 1;
        if (!exponent)
            break;
        base *= base;
    }
    return result;
}


uint64_t make_mask(unsigned width, unsigned lsb)
{
    uint64_t mask = ~0;
    /* mask is now leading 0's with 1's at the end */
    mask >>= (MAX_SHIFT - width);
    /* mask is now in the right spot */
    mask <<= lsb;
    return mask;
}
