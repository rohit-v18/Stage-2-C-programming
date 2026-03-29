#include <stdio.h>
#include <stdbool.h>
typedef enum { MODE_SLEEP, MODE_IDLE, MODE_RUN, MODE_BOOST } Mode;
int mode_to_cpu_mhz(Mode mode, bool power_save)
{
    int freq;
    switch (mode) {
        case MODE_SLEEP: return 0;
        case MODE_IDLE:  freq = 24;  break;
        case MODE_RUN:   freq = 48;  break;
        case MODE_BOOST: freq = 96;  break;
        default:         return -1;
    }
    return power_save ? freq / 2 : freq;
}
int main(void)
{
    struct {
        Mode mode;
        bool power_save;
        int  exp;
        const char *note;
    } tests[] = {
        { MODE_SLEEP,        false,  0, "Sleep clock off"                  },
        { MODE_IDLE,         false, 24, "Base frequency"                   },
        { MODE_RUN,          true,  24, "48 / 2 = 24 (power-save)"        },
        { MODE_BOOST,        true,  48, "96 / 2 = 48 (power-save)"        },
        { (Mode)99,          false, -1, "Invalid enum"                     },
        { MODE_SLEEP,        true,   0, "Remains zero even with power-save"},
    };
    int n = (int)(sizeof tests / sizeof tests[0]);
    int passed = 0;
    printf("%-4s %-38s %-8s %-8s %-6s\n", "Test", "Note", "Exp", "Got", "Result");
    printf("%.70s\n", "----------------------------------------------------------------------");
    for (int i = 0; i < n; i++) {
        int got = mode_to_cpu_mhz(tests[i].mode, tests[i].power_save);
        int ok  = (got == tests[i].exp);
        printf("%-4d %-38s %-8d %-8d %-6s\n",
               i + 1, tests[i].note, tests[i].exp, got, ok ? "PASS" : "FAIL");
        if (ok) passed++;
    }
    printf("%.70s\n", "----------------------------------------------------------------------");
    printf("Result: %d / %d tests passed.\n", passed, n);
    return (passed == n) ? 0 : 1;
}