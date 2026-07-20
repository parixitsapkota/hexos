#include <stdint.h>

uint64_t __udivdi3(uint64_t dividend, uint64_t divisor) {
    if (divisor == 0) return 0;
    if (divisor > dividend) return 0;
    if (divisor == dividend) return 1;

    // Count leading zeros to see how far we can shift the divisor
    int dividend_leading_zeros = __builtin_clzll(dividend);
    int divisor_leading_zeros = __builtin_clzll(divisor);
    
    // How many bit positions separate the dividend and divisor
    int shift = divisor_leading_zeros - dividend_leading_zeros;

    uint64_t quotient = 0;
    uint64_t remainder = dividend;

    // Align the divisor with the highest bits of the dividend
    divisor <<= shift;

    // Run the loop only for the active bit window
    for (int i = 0; i <= shift; i++) {
        quotient <<= 1;
        if (remainder >= divisor) {
            remainder -= divisor;
            quotient |= 1;
        }
        divisor >>= 1;
    }

    return quotient;
}
