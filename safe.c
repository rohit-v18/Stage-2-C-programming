#include <stdio.h>
#include <stdbool.h>
int consume_below_threshold(const int *a, int n, int *index,
                            int threshold, int limit,
                            int *out_sum, int *out_count)
{
    if (a == NULL || index == NULL || out_sum == NULL || out_count == NULL) return -1;
    if (n < 0 || limit < 0)   return -1;
    if (*index < 0 || *index > n) return -1;
    int sum = 0;
    int cnt = 0;
    while (cnt < limit && *index < n && a[*index] < threshold) {
        sum += a[(*index)++];
        cnt++;
    }
    *out_sum   = sum;
    *out_count = cnt;
    return 0;
}
int main(void)
{
    int arr1[] = {1, 2, 3, 9, 1};
    int arr2[] = {-1, -2, -3};
    int arr3[] = {1, 2, 3};
    int arr4[] = {4, 4, 4};
    int arr5[] = {1, 100, 2};
    int arr6[] = {1, 2, 3};
    int arr7[] = {1};
    struct {
        const int *a;
        int        n;
        int        start_idx;
        int        threshold;
        int        limit;
        int        exp_ret;
        int        exp_sum;
        int        exp_count;
        int        exp_idx;
        const char *note;
    } tests[] = {
        { arr1, 5, 0,  5, 10,  0,  6, 3, 3, "Stops at value >= threshold (9)"      },
        { arr2, 3, 0,  0,  2,  0, -3, 2, 2, "Limit reached early"                  },
        { arr3, 3, 3, 10,  5,  0,  0, 0, 3, "Index already at end"                 },
        { arr4, 3, 0,  4,  5,  0,  0, 0, 0, "Values equal to threshold not consumed"},
        { arr5, 3, 0, 50,  5,  0,  1, 1, 1, "Stops when next value >= threshold"   },
        { arr6, 3, 0,  5,  0,  0,  0, 0, 0, "Zero limit: consume none"             },
        { NULL, 3, 0,  5,  1, -1,  0, 0, 0, "NULL array pointer"                   },
        { arr7,-1, 0,  5,  1, -1,  0, 0, 0, "Invalid length n = -1"                },
    };
    int n = (int)(sizeof tests / sizeof tests[0]);
    int passed = 0;
    printf("%-4s %-42s %-8s %-8s %-6s\n", "Test", "Note", "Exp.Ret", "Got.Ret", "Result");
    printf("%.70s\n", "----------------------------------------------------------------------");
    for (int i = 0; i < n; i++) {
        int idx   = tests[i].start_idx;
        int sum   = -999;
        int count = -999;
        int ret   = consume_below_threshold(
                        tests[i].a, tests[i].n, &idx,
                        tests[i].threshold, tests[i].limit,
                        &sum, &count);
        int ok = (ret == tests[i].exp_ret);
        if (ok && ret == 0)
            ok = (sum   == tests[i].exp_sum)   &&
                 (count == tests[i].exp_count)  &&
                 (idx   == tests[i].exp_idx);
        printf("%-4d %-42s %-8d %-8d %-6s\n",
               i + 1, tests[i].note, tests[i].exp_ret, ret,
               ok ? "PASS" : "FAIL");
        if (ok) passed++;
    }
    printf("%.70s\n", "----------------------------------------------------------------------");
    printf("Result: %d / %d tests passed.\n", passed, n);
    return (passed == n) ? 0 : 1;
}