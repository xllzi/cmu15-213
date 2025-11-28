/* 
 * CS:APP Data Lab 
 * 
 * @xllzi
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
  3. Has unpredictable behavior when shifting if the shift amount
     is less than 0 or greater than 31.


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

For the problems that require you to implement floating-point operations,
the coding rules are less strict.  You are allowed to use looping and
conditional control.  You are allowed to use both ints and unsigneds.
You can use arbitrary integer and unsigned constants. You can use any arithmetic,
logical, or comparison operations on int or unsigned data.

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
  2. Each function has a maximum number of operations (integer, logical,
     or comparison) that you are allowed to use for your implementation
     of the function.  The max operator count is checked by dlc.
     Note that assignment ('=') is not counted; you may use as many of
     these as you want without penalty.
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
//1
/* 
 * bitXor - x^y using only ~ and & 
 *   Example: bitXor(4, 5) = 1
 *   Legal ops: ~ &
 *   Max ops: 14
 *   Rating: 1
 */
int bitXor(int x, int y) {
  return (~((~x)&(~y))&(~(x&y)));
}
/* 
 * tmin - return minimum two's complement integer 
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 4
 *   Rating: 1
 *   Tmin即除了MSB为1，其余位都为0
 */
int tmin(void) {    
  return 1 << 31;
}
//2
/*
 * isTmax - returns 1 if x is the maximum, two's complement number,
 *     and 0 otherwise 
 *   Legal ops: ! ~ & ^ | +
 *   Max ops: 10
 *   Rating: 1
 *   在计算一个Tmax的后，实现运算`==`
 *   Tmax即除了MSB为0，其余位都为1
 *   在不使用移位的情况下运算次数最大只有10次的情况下，构造Tmax似乎不可能
 */
int isTmax(int x) {
  return !(x ^ (~(1 << 31)));
}
/* 
 * allOddBits - return 1 if all odd-numbered bits in word set to 1
 *   where bits are numbered from 0 (least significant) to 31 (most significant)
 *   Examples allOddBits(0xFFFFFFFD) = 0, allOddBits(0xAAAAAAAA) = 1
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 12
 *   Rating: 2
 *   mask even-numbered bits because it is not important
 *   and then compared to 0xAAAAAAAA
 */
int allOddBits(int x) {
  return !((x & 0xAA) ^ 0xAA) & !((x >> 8 & 0xAA) ^ 0xAA) & !((x >> 16 & 0xAA) ^ 0xAA) & !((x >> 24 & 0xAA) ^ 0xAA);
}
/* 
 * negate - return -x 
 *   Example: negate(1) = -1.
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 5
 *   Rating: 2
 *   two's complement 的整数在数轴上是排除Tmin基础上对称的
 *   所以只要取反再右移1就能找到相反数
 */
int negate(int x) {
  return (~x) + 1;
}
//3
/* 
 * isAsciiDigit - return 1 if 0x30 <= x <= 0x39 (ASCII codes for characters '0' to '9')
 *   Example: isAsciiDigit(0x35) = 1.
 *            isAsciiDigit(0x3a) = 0.
 *            isAsciiDigit(0x05) = 0.
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 15
 *   Rating: 3
 *   在不实现<=运算符的前提下
 *   判断一个数是否在一个区间内可以分解为对hexadecimal的每一位的大小的判断
 */
int isAsciiDigit(int x) {
  return !(x & (~0xFF)) & !((x & 0xF0) ^ 0x30) & !(((x & 0xF) + 6) & 0x10);
}
/* 
 * conditional - same as x ? y : z 
 *   Example: conditional(2,4,5) = 4
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 16
 *   Rating: 3
 *   我要计算中保留y和z的原值
 *   y & -1 = y
 *   y & 0 = 0
 *   若x = 1, y & -x = y, z & -(!x) = 0
 *   两者和即为y
 *   若x = 0, y & -x = 0, z & -(!x) = z
 *   两者和即为z
 *   唯一的问题是我需要映射x为0或-1
 */
int conditional(int x, int y, int z) {
  // 根据x的真假来确定mask的值（-1或0）
  // 若x为非0数，!!x = 1, !x = 0
  return (y & (~!!x + 1)) + (z & (~!x + 1));
}
/* 
 * isLessOrEqual - if x <= y  then return 1, else return 0 
 *   Example: isLessOrEqual(4,5) = 1.
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 24
 *   Rating: 3
 *   实现<=运算
 *   x <= y 等价于 y - x >= 0
 *   x >= 0 等价于x的MSB为0
 *   但是整数溢出会导致极端值不满足这一数学性质
 *   应该进一步根据符号来判断大小
 */
int isLessOrEqual(int x, int y) {
    int x_sign = (x >> 31) & 1;  // x的符号
    int y_sign = (y >> 31) & 1;  // y的符号

    int diff_sign = x_sign ^ y_sign; // x y符号是否不同
    int diff = y + (~x + 1);         // y - x
    int diff_msb = (diff >> 31) & 1; // y - x 的符号位

    // 符号不同且 x 负 → x <= y
    // 符号相同且 y-x >=0 → x <= y
    return (diff_sign & x_sign) | (!diff_sign & !diff_msb);
}
//4
/* 
 * logicalNeg - implement the ! operator, using all of 
 *              the legal operators except !
 *   Examples: logicalNeg(3) = 0, logicalNeg(0) = 1
 *   Legal ops: ~ & ^ | + << >>
 *   Max ops: 12
 *   Rating: 4 
 *   若x至少有一位1,则返回0
 *   若x为0,则返回1
 *   这两者是互补的
 *   简单的判断应是判断x是否为0
 *   0的性质：0的相反数仍是0
 *   而Tmin即0x80000000的相反数也仍是本身
 *   前者是向上溢出，后者是two's complement的wrap around
 *   所以不能使用该性质来判断一个数是否为0
 *
 *   问题是：~x + 1 ^ x 得到的值与目标值是相反的
 *   对于非0数x, 返回值又是不稳定的
 *   而我要得到目标值却恰好需要`!`——我需要实现的运算
 *   关键不是判断一个数是否为0, 而是映射一个非0数为1
 *
 *   对于任何非零的 x，x 或上 -x 的最高位必有一个为 1
 */
int logicalNeg(int x) {
  int negX = ~x + 1;
  int sign = (x | negX) >> 31;
  return sign + 1;
}
/* howManyBits - return the minimum number of bits required to represent x in
 *             two's complement
 *  Examples: howManyBits(12) = 5
 *            howManyBits(298) = 10
 *            howManyBits(-5) = 4
 *            howManyBits(0)  = 1
 *            howManyBits(-1) = 1
 *            howManyBits(0x80000000) = 32
 *  Legal ops: ! ~ & ^ | + << >>
 *  Max ops: 90
 *  Rating: 4
 *  在转换decimal为binary时，我们使用除余法
 *  逐渐mod 2，得到每一位，最后加上符号位
 *  问题是没有循环，不能模拟这个计算过程
 *
 *  参数x在底层就是以32-bits的形式表示的
 *  只需要从最高位开始排除多余的1或0即可得到最少位
 *  若x >= 0, 右移n位，x为0, 则最少位=n+1
 *  若x < 0, 右移n位，x为-1, 则最少位=n+1
 *  所以以x是否为非负数为条件，分别返回两个值
 *
 *  问题是：没有循环怎么找到n?
 *  计数所有满足条件!(rshiftb ^ check_value) == 1的变量
 *  假设从rshiftb开始，以下所有变量都满足条件，则数量为32 - b, 应返回b+1, 即32 - (32 - b) + 1
 */
int howManyBits(int x) {
  int is_not_neg = !(x & (1 << 31));
  // 若是非负值，即is_not_neg = 1，check_value = 0
  // 反之, is_not_neg = 0, check_value = -1
  int check_value = is_not_neg + (~1 + 1);
  int cnt = !(x >> 0 ^ check_value) + 
            !(x >> 1 ^ check_value) + 
            !(x >> 2 ^ check_value) + 
            !(x >> 3 ^ check_value) +
            !(x >> 4 ^ check_value) +
            !(x >> 5 ^ check_value) +
            !(x >> 6 ^ check_value) +
            !(x >> 7 ^ check_value) +
            !(x >> 8 ^ check_value) +
            !(x >> 9 ^ check_value) +
            !(x >> 10 ^ check_value) +
            !(x >> 11 ^ check_value) +
            !(x >> 12 ^ check_value) +
            !(x >> 13 ^ check_value) +
            !(x >> 14 ^ check_value) +
            !(x >> 15 ^ check_value) +
            !(x >> 16 ^ check_value) +
            !(x >> 17 ^ check_value) +
            !(x >> 18 ^ check_value) +
            !(x >> 19 ^ check_value) +
            !(x >> 20 ^ check_value) +
            !(x >> 21 ^ check_value) +
            !(x >> 22 ^ check_value) +
            !(x >> 23 ^ check_value) +
            !(x >> 24 ^ check_value) +
            !(x >> 25 ^ check_value) +
            !(x >> 26 ^ check_value) +
            !(x >> 27 ^ check_value) +
            !(x >> 28 ^ check_value) +
            !(x >> 29 ^ check_value) +
            !(x >> 30 ^ check_value) +
            !(x >> 31 ^ check_value);
  int minbits = 32 + (~cnt + 1) + 1;
  return minbits;
}
//float
/* 
 * floatScale2 - Return bit-level equivalent of expression 2*f for
 *   floating point argument f.
 *   Both the argument and result are passed as unsigned int's, but
 *   they are to be interpreted as the bit-level representation of
 *   single-precision floating point values.
 *   When argument is NaN, return argument
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. also if, while
 *   Max ops: 30
 *   Rating: 4
 *   对于exp != 0 && exp != 0xFF的情况，exp++就可以使得value * 2
 *   若exp == 0xFF - 0x1, 将frac清零即可表示向上溢出，返回inf, 即uf &= 0b11111111100000000000000000000000
 *   若exp == 0xFF, 不做任何事，inf * 2 == inf
 *   若exp == 0, 
 *      若frac << 1不溢出，即第23位不是1，那么frac << 1即可
 *      若第23位是1, 直接左移，结果normalized，数值是符合预期的算术结果的
 */
unsigned floatScale2(unsigned uf) {
  unsigned exp = uf >> 23 & 0xFF;
  unsigned frac = uf & 0xFFFFFF; // 包含第24位
  if (exp == 0xFF) {
      // for either uf is Nah or inf, return itself
  } else if (exp == 0) {
      uf = (uf & 0xFF000000) + (frac << 1);
  } else if (exp == 0xFF - 1) {
      uf = (uf + 0x800000) & 0xff800000;
  } else {
      uf += 0x800000;
  }
  return uf;
}
/* 
 * floatFloat2Int - Return bit-level equivalent of expression (int) f
 *   for floating point argument f.
 *   Argument is passed as unsigned int, but
 *   it is to be interpreted as the bit-level representation of a
 *   single-precision floating point value.
 *   Anything out of range (including NaN and infinity) should return
 *   0x80000000u.
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. also if, while
 *   Max ops: 30
 *   Rating: 4
 *   若exp == 0, 则返回0
 *   若exp == 0xFF, 则返回0x80000000u
 *   在frac == 0的情况下，uf仍可能有小数，当E < 0时
 *   当E > 31, 返回INT_MIN
 *
 *   当frac != 0, 浮点数仍有可能不为小数
 *   所以总是舍弃frac是不合理的
 *   我应该考虑的是在将 1.frac 左移 exp-bias 位后，小数部分必须正好对齐到整数边界，不留下任何“非零的右边位”
 *   若frac左移exp-bias位后为0，那么浮点数恰好表示整数
 *   若不为0, 此时的frac表示的小数应被舍去, 那么未被舍去的小数应该被加上
 *   也就是加上从左至右exp-bias位的frac，使用unsigned加法
 */
int floatFloat2Int(unsigned uf) {
  unsigned sign = uf >> 31;
  unsigned exp = uf >> 23 & 0xFF;
  unsigned frac = uf & (0xFFFFFFu >> 1);
  if (exp == 0) return 0;
  else if (exp == 0xFF) return 0x80000000u;
  else {
      int shift = (exp - (0x80 - 1));
      if (shift < 0) {
          return 0;
      } else if (shift >= 31) {
          return 0x80000000u;
      } else {
          // frac will be interpreted as unsigned prepend a 1
          // there is a conceptual floating point between the bits
          // mask frac according to the position of floating point
          frac += 1 << 23;
          if (shift < 23) {
              // mask low 23 - shift of frac
              frac = frac >> (23-shift);
              return (frac ^ (0u - sign)) + sign;
          } else {
              // 没有小数
              unsigned E = shift - 23;
              return (frac ^ (0u - sign)) + sign;
          }
      }
  }
}
/* 
 * floatPower2 - Return bit-level equivalent of the expression 2.0^x
 *   (2.0 raised to the power x) for any 32-bit integer x.
 *
 *   The unsigned value that is returned should have the identical bit
 *   representation as the single-precision floating-point number 2.0^x.
 *   If the result is too small to be represented as a denorm, return
 *   0. If too large, return +INF.
 * 
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. Also if, while 
 *   Max ops: 30 
 *   Rating: 4
 */
unsigned floatPower2(int x) {
    return 2;
}
