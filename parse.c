#include <stdio.h>
#include <string.h>
typedef enum {
    PARSE_OK               = 0,
    PARSE_OVERFLOW         = 1,
    PARSE_TIMEOUT          = 2,
    PARSE_NEED_MORE        = 3,
    PARSE_PARTIAL_NO_DELIM = 4
} ParseStatus;
int parse_record_while(const char *in, int n, int *index, char delimiter,
                       char *out, int out_capacity, int max_scan_without_delim,
                       int *out_len, ParseStatus *out_status)
{
    if (!in || !index || !out || !out_len || !out_status) return -1;
    if (n < 0 || out_capacity <= 0 || max_scan_without_delim < 0) return -1;
    if (*index < 0 || *index > n) return -1;
    int out_count = 0;
    int scanned   = 0;
    int i         = *index;
    while (i < n && scanned < max_scan_without_delim) {
        char c = in[i];
        if (c == delimiter) {
            *index     = i + 1;
            *out_len   = out_count;
            *out_status = PARSE_OK;
            return 0;
        }
        if (out_count == out_capacity) {
            *out_len    = out_count;
            *out_status = PARSE_OVERFLOW;
            return 0;
        }
        if ((c < ' ' && c != '\t' && c != '\r') || c > '~') {
            i++;
            scanned++;
            continue;
        }
        out[out_count++] = c;
        i++;
        scanned++;
    }
    *index   = i;
    *out_len = out_count;
    if (scanned >= max_scan_without_delim && i < n) {
        *out_status = PARSE_TIMEOUT;
    } else if (out_count == 0) {
        *out_status = PARSE_NEED_MORE;
    } else {
        *out_status = PARSE_PARTIAL_NO_DELIM;
    }
    return 0;
}
static const char *status_str(ParseStatus s)
{
    if (s == PARSE_OK)               return "PARSE_OK";
    if (s == PARSE_OVERFLOW)         return "PARSE_OVERFLOW";
    if (s == PARSE_TIMEOUT)          return "PARSE_TIMEOUT";
    if (s == PARSE_NEED_MORE)        return "PARSE_NEED_MORE";
    if (s == PARSE_PARTIAL_NO_DELIM) return "PARSE_PARTIAL_NO_DELIM";
    return "UNKNOWN";
}
int main(void)
{
    struct {
        const char   *in;
        int           n;
        int           start_idx;
        char          delim;
        int           out_cap;
        int           max_scan;
        int           exp_ret;
        ParseStatus   exp_status;
        int           exp_len;
        int           exp_idx;
        const char   *exp_out;
        const char   *note;
    } tests[] = {
        { "ABC\nXYZ",  7, 0, '\n', 8, 100, 0, PARSE_OK,               3, 4, "ABC",     "Delimiter found"                },
        { "ABCD",      4, 0, '\n', 3, 100, 0, PARSE_OVERFLOW,         3, 0, "ABC",     "Overflow before delimiter"      },
        { "A\tB\rC\n", 6, 0, '\n', 8, 100, 0, PARSE_OK,               5, 6, "A\tB\rC", "Tab/CR kept as printable"      },
        { "\x01" "A\x7f\n", 4, 0, '\n', 8, 100, 0, PARSE_OK,          1, 4, "A",       "Skips 0x01 and 0x7F"           },
        { "ABCDEFGHI", 9, 0, '\n', 8,   5, 0, PARSE_TIMEOUT,          5, 5, "ABCDE",   "Timeout after 5 scanned"       },
        { "",          0, 0, '\n', 8,  10, 0, PARSE_NEED_MORE,        0, 0, "",        "Empty input"                   },
        { "PARTIAL",   7, 0, '\n', 8, 100, 0, PARSE_PARTIAL_NO_DELIM, 7, 7, "PARTIAL", "Partial record at end"         },
        { NULL,        4, 0, '\n', 8,  10,-1, PARSE_OK,               0, 0, "",        "NULL in pointer"               },
        { "ABC",       3, 0, '\n', 0,  10,-1, PARSE_OK,               0, 0, "",        "out_capacity=0 invalid"        },
    };
    int n = (int)(sizeof tests / sizeof tests[0]);
    int passed = 0;
    printf("%-4s %-36s %-8s %-22s %-6s\n",
           "Test", "Note", "Exp.Ret", "Exp.Status", "Result");
    printf("%.76s\n", "----------------------------------------------------------------------------");
    for (int i = 0; i < n; i++) {
        char out[64]     = {0};
        int  idx         = tests[i].start_idx;
        int  olen        = -1;
        ParseStatus stat = PARSE_OK;
        const char *inp  = tests[i].in;
        int *idxp        = (tests[i].exp_ret == -1 && inp == NULL) ? &idx : &idx;
        int ret = parse_record_while(inp, tests[i].n, idxp, tests[i].delim,
                                     out, tests[i].out_cap, tests[i].max_scan,
                                     &olen, &stat);
        int ok = (ret == tests[i].exp_ret);
        if (ok && ret == 0) {
            ok = (stat == tests[i].exp_status) &&
                 (olen == tests[i].exp_len)    &&
                 (idx  == tests[i].exp_idx)    &&
                 (memcmp(out, tests[i].exp_out, (size_t)tests[i].exp_len) == 0);
        }
        printf("%-4d %-36s %-8d %-22s %-6s\n",
               i + 1, tests[i].note, tests[i].exp_ret,
               tests[i].exp_ret == 0 ? status_str(tests[i].exp_status) : "N/A",
               ok ? "PASS" : "FAIL");
        if (ok) passed++;
    }
    printf("%.76s\n", "----------------------------------------------------------------------------");
    printf("Result: %d / %d tests passed.\n", passed, n);
    return (passed == n) ? 0 : 1;
}