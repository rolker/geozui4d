// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

/* Include file for MT19937                                       */
/*     : Mixed Integer and Real version (2001/05/12))             */
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

/* unsigned int *saverand()                                       */
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

#ifndef RANDOM_H
#define RANDOM_H

#define RANDOM_ARRAY_SIZE	624

void seedrand(unsigned long seed);	// seed the random number generator
void seedrand(unsigned long seed_array[]); // reset the random num gens state
unsigned long *saverand();	// fetch the state of the random num generator

unsigned long intrand();	// random integer 0 <= x <= 2^32 - 1
double realrand();		// random real number 0.0 <= x <= 1.0

#endif
