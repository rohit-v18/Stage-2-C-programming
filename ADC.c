/*
 * adc_to_percent.c
 * Converts a raw ADC reading to a battery percentage using integer arithmetic.
 *
 * Concepts: Variables & Data Types, Integer Arithmetic, Input Validation
 */

#include <stdio.h>

/*
 * adc_to_percent - Converts raw ADC count to battery percentage.
 *
 * @adc        : Raw ADC count (0 .. adc_max).
 * @adc_max    : Full-scale ADC count for the hardware (e.g., 1023, 4095).
 * @out_percent: Output pointer; receives the result in the range [0, 100].
 *
 * Returns 0 on success, -1 on invalid inputs or NULL pointer.
 *
 * Algorithm:
 *   percent = (adc * 100 + adc_max / 2) / adc_max
 *
 *   Adding (adc_max / 2) before the final division implements
 *   round-to-nearest using only integer arithmetic.
 *   A 'long' intermediate prevents overflow when adc_max is large
 *   (e.g., 4095 * 100 = 409500, which fits in long but may not in int
 *   on 16-bit targets).
 */
int adc_to_percent(int adc, int adc_max, int *out_percent)
{
    /* --- Input validation -------------------------------------------- */
    if (out_percent == NULL) {
        return -1;           /* Null output pointer */
    }
    if (adc_max <= 0) {
        return -1;           /* Division by zero or nonsensical max */
    }
    if (adc < 0 || adc > adc_max) {
        return -1;           /* ADC reading out of valid range */
    }

    /* --- Percentage calculation --------------------------------------- */
    /*
     * Use 'long' to hold the intermediate product so that even a
     * 12-bit ADC (max 4095) multiplied by 100 does not overflow on
     * platforms where 'int' is 16 bits.
     */
    long product   = (long)adc * 100L;
    long half      = (long)adc_max / 2;          /* For round-to-nearest */
    long result    = (product + half) / (long)adc_max;

    /* --- Clamp to [0, 100] ------------------------------------------- */
    if (result < 0)   result = 0;
    if (result > 100) result = 100;

    /* --- Store and return --------------------------------------------- */
    *out_percent = (int)result;
    return 0;
}

/* =========================================================================
 * Self-contained test harness
 * ========================================================================= */
int main(void)
{
    /*
     * Each row: { adc, adc_max, expected_return, expected_percent, description }
     * A sentinel expected_percent of -1 means the return value is -1
     * (don't care about *out_percent in error cases).
     */
    struct {
        int  adc;
        int  adc_max;
        int  exp_ret;
        int  exp_pct;
        const char *note;
    } tests[] = {
        {    0, 4095,  0,   0, "Lower bound"                  },
        { 4095, 4095,  0, 100, "Upper bound"                  },
        { 2048, 4095,  0,  50, "Mid-scale (rounded)"          },
        { 1023, 1023,  0, 100, "adc == adc_max (10-bit)"      },
        { 1100, 1023, -1,  -1, "Invalid: adc > adc_max"       },
        {    5,    0, -1,  -1, "Invalid: adc_max == 0"        },
        {  205, 4095,  0,   5, "Rounding check (205/4095)"    },
    };

    int n = (int)(sizeof tests / sizeof tests[0]);
    int passed = 0;

    printf("%-6s %-8s %-8s %-10s %-10s %-10s %s\n",
           "Test", "adc", "adc_max", "Exp.Ret", "Exp.Pct",
           "Got.Ret", "Result");
    printf("%.70s\n", "----------------------------------------------------------------------");

    for (int i = 0; i < n; i++) {
        int pct    = -999;   /* Sentinel: "not written" */
        int ret    = adc_to_percent(tests[i].adc, tests[i].adc_max, &pct);

        /* Determine pass/fail */
        int ok = (ret == tests[i].exp_ret);
        if (ok && ret == 0) {
            ok = (pct == tests[i].exp_pct);
        }

        printf("%-6d %-8d %-8d %-10d %-10d %-10d %s\n",
               i + 1,
               tests[i].adc,
               tests[i].adc_max,
               tests[i].exp_ret,
               tests[i].exp_pct,
               ret,
               ok ? "PASS" : "FAIL");

        if (ok) passed++;
    }

    printf("%.70s\n", "----------------------------------------------------------------------");
    printf("Result: %d / %d tests passed.\n", passed, n);

    return (passed == n) ? 0 : 1;
}