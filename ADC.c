#include <stdio.h>
int adc_to_percent(int adc, int adc_max, int *out_percent)
{
    if (out_percent == NULL) {
        return -1;
    }
    if (adc_max <= 0) {
        return -1;
    }
    if (adc < 0 || adc > adc_max) {
        return -1;
    }
    long product   = (long)adc * 100L;
    long half      = (long)adc_max / 2;
    long result    = (product + half) / (long)adc_max;
    if (result < 0)   result = 0;
    if (result > 100) result = 100;
    *out_percent = (int)result;
    return 0;
}
int main(void)
{
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
        int pct    = -999;
        int ret    = adc_to_percent(tests[i].adc, tests[i].adc_max, &pct);
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
