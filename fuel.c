#include <stdio.h>
#include <math.h>
int compute_kmpl(int distance_meters, int fuel_milliliters, double *out_kmpl)
{
    if (out_kmpl == NULL)        return -1;
    if (distance_meters < 0)     return -1;
    if (fuel_milliliters <= 0)   return -1;
    *out_kmpl = (distance_meters / 1000.0) / (fuel_milliliters / 1000.0);
    return 0;
}
int main(void)
{
    struct {
        int    dist;
        int    fuel;
        double exp_kmpl;
        int    exp_ret;
        int    use_null;
        const char *note;
    } tests[] = {
        {     0,   500,  0.0,   0, 0, "Zero distance => 0 km/L"          },
        { 50000,  2500, 20.0,   0, 0, "50 km / 2.5 L = 20.0"            },
        { 12345,   678, 18.21,  0, 0, "FP precedence/precision check"    },
        {  1000,     0,  0.0,  -1, 0, "Invalid: fuel = 0 (div by zero)"  },
        {   -10,   100,  0.0,  -1, 0, "Invalid: negative distance"       },
        {  1000,  1000,  0.0,  -1, 1, "Null output pointer"              },
        {   999,  1000,  0.999, 0, 0, "Sub-km distance FP correctness"   },
    };
    int n = (int)(sizeof tests / sizeof tests[0]);
    int passed = 0;
    printf("%-4s %-36s %-8s %-8s %-6s\n", "Test", "Note", "Exp.Ret", "Got.Ret", "Result");
    printf("%.65s\n", "-----------------------------------------------------------------");
    for (int i = 0; i < n; i++) {
        double kmpl = -999.0;
        double *ptr = tests[i].use_null ? NULL : &kmpl;
        int ret = compute_kmpl(tests[i].dist, tests[i].fuel, ptr);
        int ok = (ret == tests[i].exp_ret);
        if (ok && ret == 0)
            ok = (fabs(kmpl - tests[i].exp_kmpl) < 0.01);
        printf("%-4d %-36s %-8d %-8d %-6s\n",
               i + 1, tests[i].note, tests[i].exp_ret, ret,
               ok ? "PASS" : "FAIL");
        if (ok) passed++;
    }
    printf("%.65s\n", "-----------------------------------------------------------------");
    printf("Result: %d / %d tests passed.\n", passed, n);
    return (passed == n) ? 0 : 1;
}