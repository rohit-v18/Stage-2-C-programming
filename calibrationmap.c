#include <stdio.h>
#include <stdbool.h>
int calib_map_linear(int x, int x1, int y1, int x2, int y2,
                     bool clamp, int y_min, int y_max, int *out_y)
{
    if (out_y == NULL)          return -1;
    if (x1 == x2)               return -1;
    if (clamp && y_min > y_max) return -1;
    long num = (long)(x - x1) * (long)(y2 - y1);
    long den = (long)(x2 - x1);
    long adj = (num >= 0) ? (den / 2) : -(den / 2);
    long term = (num + adj) / den;
    long y_long = (long)y1 + term;
    if (clamp) {
        if (y_long < y_min) y_long = y_min;
        else if (y_long > y_max) y_long = y_max;
    }
    *out_y = (int)y_long;
    return 0;
}
int main(void)
{
    struct {
        int  x, x1, y1, x2, y2;
        bool clamp;
        int  y_min, y_max;
        int  exp_ret, exp_y;
        const char *note;
    } tests[] = {
        { 75,  50,   0, 100, 100, false,   0,   0,  0, 50,  "Midpoint maps to 50"         },
        {100,  50,   0, 100, 100, false,   0,   0,  0, 100, "Exact endpoint"              },
        {120,  50,   0, 100, 100, true,    0, 100,  0, 100, "Extrapolation clamped high"  },
        { 40,  50,   0, 100, 100, true,    0, 100,  0,   0, "Extrapolation clamped low"   },
        { 60, 100, 200, 200, 400, false,   0,   0,  0, 120, "Extrapolation (rounded)"     },
        { 10,  10, 500,  10, 900, false,   0,   0, -1,  -1, "Degenerate: x1 == x2"       },
        {  0,  50,   0, 100, 100, false,   0,   0, -1,  -1, "NULL out_y pointer"         },
    };
    int n = (int)(sizeof tests / sizeof tests[0]);
    int passed = 0;
    printf("%-4s %-35s %-10s %-10s %-10s %-6s\n",
           "Test", "Note", "Exp.Ret", "Exp.Y", "Got.Ret", "Result");
    printf("%.80s\n", "--------------------------------------------------------------------------------");
    for (int i = 0; i < n; i++) {
        int y = -999;
        int *ptr = (i == n - 1) ? NULL : &y;
        int ret = calib_map_linear(
            tests[i].x,  tests[i].x1, tests[i].y1,
            tests[i].x2, tests[i].y2,
            tests[i].clamp, tests[i].y_min, tests[i].y_max, ptr);
        int ok = (ret == tests[i].exp_ret);
        if (ok && ret == 0)
            ok = (y == tests[i].exp_y);
        printf("%-4d %-35s %-10d %-10d %-10d %-6s\n",
               i + 1, tests[i].note,
               tests[i].exp_ret, tests[i].exp_y, ret,
               ok ? "PASS" : "FAIL");
        if (ok) passed++;
    }
    printf("%.80s\n", "--------------------------------------------------------------------------------");
    printf("Result: %d / %d tests passed.\n", passed, n);
    return (passed == n) ? 0 : 1;
}