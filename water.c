#include <stdio.h>
#include <stdbool.h>
int decide_pump_command(int level_pct, bool pump_running,
                        bool manual_on, bool manual_off,
                        bool leak, bool overcurrent, int *out_cmd)
{
    if (out_cmd == NULL) return -1;
    if (level_pct < 0)   level_pct = 0;
    if (level_pct > 100) level_pct = 100;
    if (leak || overcurrent)  { *out_cmd = 0; return 0; }
    if (manual_off)           { *out_cmd = 0; return 0; }
    if (manual_on)            { *out_cmd = 1; return 0; }
    if (level_pct <= 25)      { *out_cmd = 1; return 0; }
    if (level_pct >= 80)      { *out_cmd = 0; return 0; }
    *out_cmd = pump_running ? 1 : 0;
    return 0;
}
int main(void)
{
    struct {
        int  level;
        bool running;
        bool manual_on;
        bool manual_off;
        bool leak;
        bool overcurrent;
        int  use_null;
        int  exp_ret;
        int  exp_cmd;
        const char *note;
    } tests[] = {
        {  15, false, false, false, false, false, 0,  0, 1, "Below ON threshold -> START"           },
        {  85, true,  false, false, false, false, 0,  0, 0, "Above OFF threshold -> STOP"           },
        {  50, true,  false, false, false, false, 0,  0, 1, "Mid-band, hold running -> START"       },
        {  50, false, false, false, false, false, 0,  0, 0, "Mid-band, hold stopped -> STOP"        },
        {  40, false, true,  false, false, false, 0,  0, 1, "Manual ON overrides mid-band"          },
        {  10, true,  false, true,  false, false, 0,  0, 0, "Manual OFF overrides low level"        },
        {  70, true,  false, false, true,  false, 0,  0, 0, "Leak forces STOP"                     },
        { 120, false, false, false, false, false, 0,  0, 0, "Level clamped to 100 -> STOP"          },
        {  -5, false, false, false, false, false, 0,  0, 1, "Level clamped to 0 -> START"           },
        {  50, false, false, false, false, false, 1, -1, 0, "NULL out_cmd -> error"                 },
    };
    int n = (int)(sizeof tests / sizeof tests[0]);
    int passed = 0;
    printf("%-4s %-42s %-8s %-8s %-6s\n", "Test", "Note", "Exp.Ret", "Got.Ret", "Result");
    printf("%.70s\n", "----------------------------------------------------------------------");
    for (int i = 0; i < n; i++) {
        int  cmd = -999;
        int *ptr = tests[i].use_null ? NULL : &cmd;
        int  ret = decide_pump_command(tests[i].level, tests[i].running,
                                       tests[i].manual_on, tests[i].manual_off,
                                       tests[i].leak, tests[i].overcurrent, ptr);
        int ok = (ret == tests[i].exp_ret);
        if (ok && ret == 0) ok = (cmd == tests[i].exp_cmd);
        printf("%-4d %-42s %-8d %-8d %-6s\n",
               i + 1, tests[i].note, tests[i].exp_ret, ret,
               ok ? "PASS" : "FAIL");
        if (ok) passed++;
    }
    printf("%.70s\n", "----------------------------------------------------------------------");
    printf("Result: %d / %d tests passed.\n", passed, n);
    return (passed == n) ? 0 : 1;
}