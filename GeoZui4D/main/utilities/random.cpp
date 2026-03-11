// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

/* Procedures for MT19937: Integer version (1999/10/28)           */
/*                                                                */
/* C++ version with saverand() and overloaded seedrand() added,   */
/*     and function name changes done by Donald House (5/01)      */

/* unsigned long intrand() returns one pseudorandom unsigned      */
/*   integer (32bit) uniformly distributed among 0 to 2^32-1      */

/* double realrand() returns one pseudorandom real number in the  */
/*   range [0.0 ... 1.0]                                          */

/* void seedrand(int seed)                                        */
/*   sets initial values to the working area of 624 words.        */
/*   seedrand() may be called at any time to restart the random   */
/*   sequence, but normally is called only once before the first  */
/*   call to either intrand() or realrand()                       */

/* void seedrand(unsigned int seed_array[])                       */
/*   sets initial values to the working area by providing values  */
/*   in an array of 624 unsigned integers. The array must not     */
/*   contain all zeros. This version of seedrand() is called      */
/*   in order to restart the random number generator to exactly   */
/*   continue a previous sequence. (see saverand() below)         */

/* unsigned long *saverand()                                       */
/*   returns a pointer to the static array containing the random  */
/*   number generator's 624 element array of values. The data in  */
/*   this array can be saved (note, it is volatile and must be    */
/*   saved before any further calls to random number generator    */
/*   routines) and then used later in a call to seedrand() to     */
/*   restart the random sequence exactly where it left off.       */

/*   Coded by Takuji Nishimura, considering the suggestions by    */
/* Topher Cooper and Marc Rieffel in July-Aug. 1997.              */

/* This library is free software; you can redistribute it and/or   */
/* modify it under the terms of the GNU Library General Public     */
/* License as published by the Free Software Foundation; either    */
/* version 2 of the License, or (at your option) any later         */
/* version.                                                        */
/* This library is distributed in the hope that it will be useful, */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of  */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.            */
/* See the GNU Library General Public License for more details.    */
/* You should have received a copy of the GNU Library General      */
/* Public License along with this library; if not, write to the    */
/* Free Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA   */ 
/* 02111-1307  USA                                                 */

/* Copyright (C) 1997, 1999 Makoto Matsumoto and Takuji Nishimura. */
/* Any feedback is very welcome. For any question, comments,       */
/* see http://www.math.keio.ac.jp/matumoto/emt.html or email       */
/* matumoto@math.keio.ac.jp                                        */

/* REFERENCE                                                       */
/* M. Matsumoto and T. Nishimura,                                  */
/* "Mersenne Twister: A 623-Dimensionally Equidistributed Uniform  */
/* Pseudo-Random Number Generator",                                */
/* ACM Transactions on Modeling and Computer Simulation,           */
/* Vol. 8, No. 1, January 1998, pp 3--30.                          */

#include "utilities/random.h"

/* Period parameters */  
#define N RANDOM_ARRAY_SIZE
#define M 397
#define MATRIX_A 0x9908b0df   /* constant vector a */
#define UPPER_MASK 0x80000000 /* most significant w-r bits */
#define LOWER_MASK 0x7fffffff /* least significant r bits */

/* Tempering parameters */   
#define TEMPERING_MASK_B 0x9d2c5680
#define TEMPERING_MASK_C 0xefc60000
#define TEMPERING_SHIFT_U(y)  (y >> 11)
#define TEMPERING_SHIFT_S(y)  (y << 7)
#define TEMPERING_SHIFT_T(y)  (y << 15)
#define TEMPERING_SHIFT_L(y)  (y >> 18)

static unsigned long mt[N]; /* the array for the state vector  */
static int mti = N + 1; /* mti == N + 1 means mt[N] is not initialized */

/* Initializing the array with a single seed */
void seedrand(unsigned long seed)
{
  int i;
  
  for (i = 0; i < N; i++){
    mt[i] = seed & 0xffff0000;
    seed = 69069 * seed + 1;
    mt[i] |= (seed & 0xffff0000) >> 16;
    seed = 69069 * seed + 1;
  }
  mti = N;
}

/* Initialization by "seedrand()" is an example. Theoretically,      */
/* there are 2^19937-1 possible states as an intial state.           */
/* This function allows to choose any of 2^19937-1 ones.             */
/* Essential bits in "seed_array[]" is following 19937 bits:         */
/*  (seed_array[0]&UPPER_MASK), seed_array[1], ..., seed_array[N-1]. */
/* (seed_array[0]&LOWER_MASK) is discarded.                          */ 
/* Theoretically,                                                    */
/*  (seed_array[0]&UPPER_MASK), seed_array[1], ..., seed_array[N-1]  */
/* seed_array must have at least N elements that cannot be all zeros */
void seedrand(unsigned long seed_array[])
{
  int i;

  for (i = 0; i < N; i++) 
    mt[i] = seed_array[i];
  mti = N;
}

unsigned long *saverand()
{
  return mt;
}

unsigned long intrand()
{
  unsigned long y;
  static unsigned long mag01[2]={0x0, MATRIX_A};
  /* mag01[x] = x * MATRIX_A  for x=0,1 */
  
  if(mti >= N){ 	/* generate N words at one time */
    int kk;
    
    if(mti == N + 1)   	/* if sgenrand() has not been called, */
      seedrand(4357); 	/* a default initial seed is used   */
    
    for(kk = 0 ; kk < N - M; kk++){
      y = (mt[kk] & UPPER_MASK) | (mt[kk+1] & LOWER_MASK);
      mt[kk] = mt[kk + M] ^ (y >> 1) ^ mag01[y & 0x1];
    }
    for (; kk < N - 1; kk++){
      y = (mt[kk] & UPPER_MASK) | (mt[kk+1] & LOWER_MASK);
      mt[kk] = mt[kk + (M - N)] ^ (y >> 1) ^ mag01[y & 0x1];
    }
    y = (mt[N-1] & UPPER_MASK) | (mt[0] & LOWER_MASK);
    mt[N - 1] = mt[M - 1] ^ (y >> 1) ^ mag01[y & 0x1];
    
    mti = 0;
  }
  
  y = mt[mti++];
  y ^= TEMPERING_SHIFT_U(y);
  y ^= TEMPERING_SHIFT_S(y) & TEMPERING_MASK_B;
  y ^= TEMPERING_SHIFT_T(y) & TEMPERING_MASK_C;
  y ^= TEMPERING_SHIFT_L(y);
  
  return y; 
}

double realrand()
{
  return ((double)intrand() * 2.3283064370807974e-10 ); /* reals */
}
