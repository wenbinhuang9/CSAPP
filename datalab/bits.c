/* 
 * CS:APP Data Lab 
 * 
 * <Please put your name and userid here>
 * 
 * bits.c - Source file with your solutions to the Lab.
 *          This is the file you will hand in to your instructor.
 *
 * WARNING: Do not include the <stdio.h> header; it confuses the dlc
 * compiler. You can still use printf for debugging without including
 * <stdio.h>, although you might get a compiler warning. In general,
 * it's not good practice to ignore compiler warnings, but in this
 * case it's OK.  
 */

#if 0
/*
 * Instructions to Students:
 *
 * STEP 1: Read the following instructions carefully.
 */

You will provide your solution to the Data Lab by
editing the collection of functions in this source file.

INTEGER CODING RULES:
 
  Replace the "return" statement in each function with one
  or more lines of C code that implements the function. Your code 
  must conform to the following style:
 
  int Funct(arg1, arg2, ...) {
      /* brief description of how your implementation works */
      int var1 = Expr1;
      ...
      int varM = ExprM;

      varJ = ExprJ;
      ...
      varN = ExprN;
      return ExprR;
  }

  Each "Expr" is an expression using ONLY the following:
  1. Integer constants 0 through 255 (0xFF), inclusive. You are
      not allowed to use big constants such as 0xffffffff.
  2. Function arguments and local variables (no global variables).
  3. Unary integer operations ! ~
  4. Binary integer operations & ^ | + << >>
    
  Some of the problems restrict the set of allowed operators even further.
  Each "Expr" may consist of multiple operators. You are not restricted to
  one operator per line.

  You are expressly forbidden to:
  1. Use any control constructs such as if, do, while, for, switch, etc.
  2. Define or use any macros.
  3. Define any additional functions in this file.
  4. Call any functions.
  5. Use any other operations, such as &&, ||, -, or ?:
  6. Use any form of casting.
  7. Use any data type other than int.  This implies that you
     cannot use arrays, structs, or unions.

 
  You may assume that your machine:
  1. Uses 2s complement, 32-bit representations of integers.
  2. Performs right shifts arithmetically.
  3. Has unpredictable behavior when shifting an integer by more
     than the word size.

EXAMPLES OF ACCEPTABLE CODING STYLE:
  /*
   * pow2plus1 - returns 2^x + 1, where 0 <= x <= 31
   */
  int pow2plus1(int x) {
     /* exploit ability of shifts to compute powers of 2 */
     return (1 << x) + 1;
  }

  /*
   * pow2plus4 - returns 2^x + 4, where 0 <= x <= 31
   */
  int pow2plus4(int x) {
     /* exploit ability of shifts to compute powers of 2 */
     int result = (1 << x);
     result += 4;
     return result;
  }

FLOATING POINT CODING RULES

For the problems that require you to implent floating-point operations,
the coding rules are less strict.  You are allowed to use looping and
conditional control.  You are allowed to use both ints and unsigneds.
You can use arbitrary integer and unsigned constants.

You are expressly forbidden to:
  1. Define or use any macros.
  2. Define any additional functions in this file.
  3. Call any functions.
  4. Use any form of casting.
  5. Use any data type other than int or unsigned.  This means that you
     cannot use arrays, structs, or unions.
  6. Use any floating point data types, operations, or constants.


NOTES:
  1. Use the dlc (data lab checker) compiler (described in the handout) to 
     check the legality of your solutions.
  2. Each function has a maximum number of operators (! ~ & ^ | + << >>)
     that you are allowed to use for your implementation of the function. 
     The max operator count is checked by dlc. Note that '=' is not 
     counted; you may use as many of these as you want without penalty.
  3. Use the btest test harness to check your functions for correctness.
  4. Use the BDD checker to formally verify your functions
  5. The maximum number of ops for each function is given in the
     header comment for each function. If there are any inconsistencies 
     between the maximum ops in the writeup and in this file, consider
     this file the authoritative source.

/*
 * STEP 2: Modify the following functions according the coding rules.
 * 
 *   IMPORTANT. TO AVOID GRADING SURPRISES:
 *   1. Use the dlc compiler to check that your solutions conform
 *      to the coding rules.
 *   2. Use the BDD checker to formally verify that your solutions produce 
 *      the correct answers.
 */


#endif
/* 
 * bitAnd - x&y using only ~ and | 
 *   Example: bitAnd(6, 5) = 4
 *   Legal ops: ~ |
 *   Max ops: 8
 *   Rating: 1
 */
int bitAnd(int x, int y) {
  return ~(~x | ~y);
}
/* 
 * getByte - Extract byte n from word x
 *   Bytes numbered from 0 (LSB) to 3 (MSB)
 *   Examples: getByte(0x12345678,1) = 0x56
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 6
 *   Rating: 2
 */
int getByte(int x, int n) {
  int right_shift_length = n << 3; 
  int result = x >> right_shift_length;
  return result & 0xFF;
}
/* 
 * logicalShift - shift x to the right by n, using a logical shift
 *   Can assume that 0 <= n <= 31
 *   Examples: logicalShift(0x87654321,4) = 0x08765432
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 20
 *   Rating: 3 
 */
int logicalShift(int x, int n) {
  /* if positive, just shift to right n
   * if negative, we should add addtional value
  */
  int left_shift = 32 + (~n + 1);
  int sign = (x >> 31) & 0x1; 
  
  return (x >> n)  + ((sign & (!!n)) << left_shift); 
}
/*
 * bitCount - returns count of number of 1's in word
 *   Examples: bitCount(5) = 2, bitCount(7) = 3
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 40
 *   Rating: 4
 */
int bitCount(int x) {
  /*
   *  The basic idea is based on divide and conquer.
   *  1. Treat each 2 bits as a whole, plus lower one bit with higher one bit. 
   *  2. Treat each 4 bits as a whole, plus lower two bits with higher two bits. 
   */
  
  int temp_mask1 = 0x55 | (0x55 << 8); 

  int mask1 = temp_mask1 | (temp_mask1 << 16); 
  int temp_mask2 = 0x33 | (0x33 << 8);
  int maks2 = temp_mask2 | (temp_mask2 << 16); 

  int temp_mask3 = 0x0f | (0x0f << 8); 
  int mask3 = temp_mask3  | (temp_mask3 << 16);
  int mask4 =  0xff | (0xff << 16);
  int mask5 = 0xff | (0xff << 8); 

  int ans;
  ans  = (x & mask1) + ( (x >> 1) & mask1);
  ans = (ans & maks2) +( (ans >> 2) & maks2); 
  ans = (ans & mask3) +( (ans >> 4) & mask3); 
  ans = (ans & mask4) + ((ans >> 8) & mask4);  
  ans = (ans & mask5) + ((ans >> 16) & mask5); 
  return ans; 
}
/* 
 * bang - Compute !x without using !
 *   Examples: bang(3) = 0, bang(0) = 1
 *   Legal ops: ~ & ^ | + << >>
 *   Max ops: 12
 *   Rating: 4 
 */
int bang(int x) {
  // either x is positive or x - 1 is positive .
  // why we need to judge x - 1 here, because 0 is not positive , so we need to judge x - 1
  return ~(((x >> 31) & 0x1)|((~x + 1) >> 31 & 0x1)) & 1;
}
/* 
 * tmin - return minimum two's complement integer 
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 4
 *   Rating: 1
 */
int tmin(void) {
  return 1 << 31;
}
/* 
 * fitsBits - return 1 if x can be represented as an 
 *  n-bit, two's complement integer.
 *   1 <= n <= 32
 *   Examples: fitsBits(5,3) = 0, fitsBits(-4,3) = 1
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 15
 *   Rating: 2
 */
int fitsBits(int x, int n) {
  // if x is positive, 1 is only occur within n-1 bits 
  // if x is negative, 0 is only occur within n-1 bits 

  int mask = 1 << 31; 
  int sign = (x >> 31) & 0x1;
  int n_mins_1 = n + (~1 + 1);
  int positiveRes = (!sign) & (!(x >> n_mins_1));
  int negativeRes = (sign) & !(~(x>> n_mins_1)); 

  return positiveRes | negativeRes;
}
/* 
 * divpwr2 - Compute x/(2^n), for 0 <= n <= 30
 *  Round toward zero
 *   Examples: divpwr2(15,1) = 7, divpwr2(-33,4) = -2
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 15
 *   Rating: 2
 */

int divpwr2(int x, int n) {
    // if x is positive, just right shift n 
    // if x is negative and x % 2^n != 0, which means we need to do add 1 to the result. 
    int result = x >> n; 
    int mask = 0x1; 
    int is_negative = x >> 31 & mask; 
    int not_zero = !(!n); 
    int not_same =  !!(x ^ ( (x >> n) << n )); 
    int addition = is_negative & not_zero & not_same;
    return result + addition;
}
/* 
 * negate - return -x 
 *   Example: negate(1) = -1.
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 5
 *   Rating: 2
 */
int negate(int x) {
  return ~x + 1;
}
/* 
 * isPositive - return 1 if x > 0, return 0 otherwise 
 *   Example: isPositive(-1) = 0.
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 8
 *   Rating: 3
 */
int isPositive(int x) {
  // it is positive only when both x and x - 1 is >=0 
  return ((x >> 31)^0x1) & (!!x); 
}


/* 
 * isLessOrEqual - if x <= y  then return 1, else return 0 
 *   Example: isLessOrEqual(4,5) = 1.
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 24
 *   Rating: 3
 */

int isLessOrEqual(int x, int y) {
  // consider overflow of x - y, so we need to judge sign in advance. 
  // for example, if x is negative, y is positive, we just return 1 in advance. 
  int xsign = (x >> 31) &0x1;
  int ysign = (y >> 31) & 0x1;

  int diff_sign = ((x + ~y) >> 31) & 0x1; 

  return (xsign & (!ysign)) | ((!((!xsign) & ysign)) & diff_sign );
}
/*
 * ilog2 - return floor(log base 2 of x), where x > 0
 *   Example: ilog2(16) = 4
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 90
 *   Rating: 4
 */
int ilog2(int x) {
  // Get the position of highest bit. 
  // The maximum return value is 32, so the ans can be represented by ans = 16 * a + 8 * b + 4 * c + 2 * d + e
  // The basical idea is using binary search 
  // 1. divide into two parts ,each part 16 bits, if left part is > 0, it means is highest bit position is at least at 16 th bit. 
  // 1. divide into two parts ,each part is 8 bits. 
  int ans = 0; 
  
  ans +=  (!!(x >> 16)) << 4;
  ans += (!! (x >> (ans + 8))) << 3;
  ans += (!! (x >> (ans + 4))) << 2;
  ans += (!! (x >> (ans + 2))) << 1;
  ans += (!! (x >> (ans + 1))) << 0;

  return ans;
}
/* 
 * float_neg - Return bit-level equivalent of expression -f for
 *   floating point argument f.
 *   Both the argument and result are passed as unsigned int's, but
 *   they are to be interpreted as the bit-level representations of
 *   single-precision floating point values.
 *   When argument is NaN, return argument.
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. also if, while
 *   Max ops: 10
 *   Rating: 2
 */
unsigned float_neg(unsigned uf) {
  // Consider Nan 
  int exp = (uf >> 23) & 0xff;
  int frac = uf & 0x7fffff;
  int s = (uf >> 31) & 0x1;

  if (exp == 0xff && frac != 0) {
    return uf;
  }

  return (1 << 31) + uf;
}
/* 
 * float_i2f - Return bit-level equivalent of expression (float) x
 *   Result is returned as unsigned int, but
 *   it is to be interpreted as the bit-level representation of a
 *   single-precision floating point values.
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. also if, while
 *   Max ops: 30
 *   Rating: 4
 */
// todo implement it here 
unsigned float_i2f(int x) {
  // 1. consider special case 0x00000000 and 0x80000000
  // 2. consider even nearest round 
  // 3. When # of frac exceeds 23 bits, truncate the least significant part.
  // 4. when # of frac smaller than 23 bits, shift to left. 
  int s = (x >> 31) & 0x1;
  int positive_x = x >=0 ? x : -x;
  int highest_bit_pos = -1;
  int exp;
  int frac;
  int ans; 
  int i = 0;
  int round = 0;
  int truncated;
  int tempfrac;
  if (x == 0) { 
   return 0;
  }
  if (x == 0x80000000) {
    return 0xcf000000;
  }
  for (i = 0 ; i < 31; i++ ) {
    if (((positive_x >> i) & 0x1) == 1) {
      highest_bit_pos = i; 
    }
  }
 // printf("x is %d : highest_bit_pos is :%d\n", x, highest_bit_pos);
  //todo how to deal with no highest biti? 
  exp = 127 + highest_bit_pos; 
  // compute Frac 
  frac = positive_x ^ (1 << highest_bit_pos); 
  
  if (highest_bit_pos - 23 > 0) {
    // truncate
     int truncated_len = highest_bit_pos -23;

     tempfrac = (frac >> truncated_len);
     truncated=  (tempfrac << truncated_len) ^ frac;
     frac = tempfrac;
     if (truncated > (1<< (truncated_len - 1))) {
        round = 1;
     } 
     if (truncated == (1 << (truncated_len - 1)) && ((frac & 1) == 1)) {
        // round to even 
        round = 1;
     }
   //  printf("truncated is %d, frac is %d, round is %d", truncated, frac, round);
     frac += round; 
  } else {
    int right_shift_len = 23 - highest_bit_pos;
    frac = (frac << right_shift_len); 
  }
  
  ans = (s << 31) + (exp << 23) + frac; 

  // consider overflow, detect overflow 
  return ans;
}
/* 
 * float_twice - Return bit-level equivalent of expression 2*f for
 *   floating point argument f.
 *   Both the argument and result are passed as unsigned int's, but
 *   they are to be interpreted as the bit-level representation of
 *   single-precision floating point values.
 *   When argument is NaN, return argument
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. also if, while
 *   Max ops: 30
 *   Rating: 4
 */
unsigned float_twice(unsigned uf) {
  // if normalized value , add exp by 1
  // if denormalized, right shift to 1
  int tmp=uf;
  int sign=((uf>>31)<<31); /* 0x80000000 or 0x0 */
  int exp=uf&0x7f800000;
  int frac=uf&0x7fffff;
  tmp=tmp&0x7fffffff; /* remove sign */


  if ((exp  >> 23) == 0xff || (exp == 0 && frac == 0)) {
    // nan or infinity 
    return uf; 
  }
  if ( exp == 0) {
   // denormalized value 
    return sign + exp + (frac << 1); 
  }
  // exp != 0  and != 0xff, it is normalized value
  exp += (1 << 23) ;
  if (exp == 0xff) {
    //infinity
    return 0xff << 23;
  }
  return sign + exp + frac;
}
