#include <stdio.h>
#include <string.h>
int drain_ring_snapshot(const char *buf, int capacity,
                        int read_index_snapshot, int write_index_snapshot,
                        int max_to_copy, char *out, int *out_count)
{
    if (buf == NULL || out == NULL || out_count == NULL) return -1;
    if (capacity <= 0 || max_to_copy < 0)               return -1;
    if (read_index_snapshot  < 0 || read_index_snapshot  >= capacity) return -1;
    if (write_index_snapshot < 0 || write_index_snapshot >= capacity) return -1;
    int i   = 0;
    int idx = read_index_snapshot;
    for (i = 0, idx = read_index_snapshot;
         i < max_to_copy && idx != write_index_snapshot;
         i++, idx = (idx + 1) % capacity)
        out[i] = buf[idx];
    *out_count = i;
    return 0;
}
int main(void)
{
    const char *buf8 = "ABCDEFGH";
    struct {
        const char *buf;
        int  capacity;
        int  read;
        int  write;
        int  max;
        int  exp_ret;
        int  exp_count;
        const char *exp_out;
        const char *note;
    } tests[] = {
        { buf8, 8, 2, 6, 10,  0, 4, "CDEF", "No wrap; stops at write snapshot"     },
        { buf8, 8, 6, 2, 10,  0, 4, "GHAB", "Wrap-around (6->7->0->1)"             },
        { buf8, 8, 3, 3,  5,  0, 0, "",     "Empty snapshot (read == write)"        },
        { buf8, 8, 1, 5,  2,  0, 2, "BC",   "Quota-limited (2 < available 4)"      },
        { buf8, 8, 7, 0,  5,  0, 1, "H",    "Single element; wrap stops at 0"      },
        { buf8, 8, 0, 5,  0,  0, 0, "",     "Zero quota"                            },
        { "X",  1, 0, 0,  3,  0, 0, "",     "Empty snapshot in 1-slot ring"        },
        { buf8, 8, 8, 0,  3, -1, 0, "",     "Index out of range (read=8)"          },
        { buf8, 0, 0, 0,  3, -1, 0, "",     "Invalid capacity (0)"                 },
        { NULL, 8, 0, 0,  3, -1, 0, "",     "NULL buf pointer"                     },
    };
    int n = (int)(sizeof tests / sizeof tests[0]);
    int passed = 0;
    printf("%-4s %-38s %-8s %-10s %-6s\n",
           "Test", "Note", "Exp.Ret", "Exp.Count", "Result");
    printf("%.68s\n", "--------------------------------------------------------------------");
    for (int i = 0; i < n; i++) {
        char out[16] = {0};
        int  count   = -1;
        int  ret     = drain_ring_snapshot(tests[i].buf, tests[i].capacity,
                                           tests[i].read, tests[i].write,
                                           tests[i].max, out, &count);
        int ok = (ret == tests[i].exp_ret);
        if (ok && ret == 0)
            ok = (count == tests[i].exp_count) &&
                 (memcmp(out, tests[i].exp_out, (size_t)tests[i].exp_count) == 0);
        printf("%-4d %-38s %-8d %-10d %-6s\n",
               i + 1, tests[i].note, tests[i].exp_ret,
               tests[i].exp_count, ok ? "PASS" : "FAIL");
        if (ok) passed++;
    }
    printf("%.68s\n", "--------------------------------------------------------------------");
    printf("Result: %d / %d tests passed.\n", passed, n);
    return (passed == n) ? 0 : 1;
}