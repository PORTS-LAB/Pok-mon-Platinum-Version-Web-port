/* Known-answer test for real decomp code (pokeplatinum src/ascii_util.c).
 * Built twice: emcc->wasm (run under node) and host gcc -m32.
 * Outputs must match byte-for-byte (verified 2026-10-16: IDENTICAL). */
#include <stdio.h>
#include "ascii_util.h"

int main(void) {
    printf("len_empty=%d\n", Ascii_Length(""));
    printf("len_hello=%d\n", Ascii_Length("hello"));
    printf("atoi_123=%d\n", Ascii_ConvertToInt("123"));
    printf("atoi_neg=%d\n", Ascii_ConvertToInt("-45"));
    printf("atoi_bad=%d\n", Ascii_ConvertToInt("12a4"));
    printf("fullwidth_A=%d\n", IsFullWidthChar('A'));
    printf("fullwidth_hi=%d\n", IsFullWidthChar(0x100));
    char dst[300];
    char src[] = "ab\rcd";
    char *rest = Ascii_CopyToTerminator(src, dst, '\r');
    printf("copy=%s rest=%s\n", dst, rest);
    printf("sqrt9=%d sqrt2=%d div=%d rem=%d\n",
        (int)SVC_Sqrt(9), (int)SVC_Sqrt(2),
        (int)SVC_Div(7, 3), (int)SVC_DivRem(7, 3));
    return 0;
}
