#include <stdio.h>
int count_above_threshold(const int *a, int n, int threshold, int *out_count)
{
    if (a == NULL || out_count == NULL || n < 0) return -1;
    int count = 0;
    for (int i = 0; i < n; i++)
        if (a[i] > threshold) count++;
    *out_count = count;
    return 0;
}
int main(void)
{
    int arr1[] = {1, 5, 9};
    int arr2[] = {10, 10, 10};
    int arr3[] = {-1, -5, -3};
    int dummy  = 0;
    struct {
        const int *a;
        int        n;
        int        threshold;
        int       *out;
        int        exp_ret;
        int        exp_count;
        const char *note;
    } tests[] = {
        { arr1, 3,  4, &dummy,  0, 2, "5 & 9 above 4"            },
        { arr2, 3, 10, &dummy,  0, 0, "Strictly greater: none"   },
        { arr3, 3, -4, &dummy,  0, 2, "-1 & -3 exceed -4"        },
        { arr1, 0,  5, &dummy,  0, 0, "Empty array (n=0)"        },
        { NULL, 3,  5, &dummy, -1, 0, "NULL array pointer"        },
        { arr1, 3,  5,  NULL,  -1, 0, "NULL out_count pointer"   },
        { arr1,-1,  5, &dummy, -1, 0, "Invalid size n=-1"        },
    };
    int n = (int)(sizeof tests / sizeof tests[0]);
    int passed = 0;
    printf("%-4s %-28s %-8s %-10s %-8s %-6s\n",
           "Test", "Note", "Exp.Ret", "Exp.Count", "Got.Ret", "Result");
    printf("%.66s\n", "------------------------------------------------------------------");
    for (int i = 0; i < n; i++) {
        dummy = -999;
        int ret = count_above_threshold(tests[i].a, tests[i].n,
                                        tests[i].threshold, tests[i].out);
        int ok = (ret == tests[i].exp_ret);
        if (ok && ret == 0) ok = (dummy == tests[i].exp_count);
        printf("%-4d %-28s %-8d %-10d %-8d %-6s\n",
               i + 1, tests[i].note,
               tests[i].exp_ret, tests[i].exp_count, ret,
               ok ? "PASS" : "FAIL");
        if (ok) passed++;
    }
    printf("%.66s\n", "------------------------------------------------------------------");
    printf("Result: %d / %d tests passed.\n", passed, n);
    return (passed == n) ? 0 : 1;
}
