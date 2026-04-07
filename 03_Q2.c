#include <stdio.h>
#include <stdbool.h>
typedef enum {
    REASON_ACCEPTED     = 0,
    REASON_MAINTENANCE  = 1,
    REASON_COOLDOWN     = 2,
    REASON_MIN_GAP      = 3,
    REASON_WINDOW_LIMIT = 4
} AdmissionReason;
int decide_admission(long now_ms,
                     long *window_start_ms, int window_ms, int max_in_window,
                     long min_gap_ms, long cooldown_ms,
                     long *last_accept_ms, long *cooldown_until_ms,
                     int *count_in_window,
                     bool maintenance_mode, bool emergency_override,
                     int *out_decision, AdmissionReason *out_reason)
{
    if (!window_start_ms || !last_accept_ms || !cooldown_until_ms ||
        !count_in_window  || !out_decision  || !out_reason)
        return -1;
    if (now_ms < 0 || *window_start_ms < 0 || window_ms <= 0 ||
        max_in_window < 1 || min_gap_ms < 0 || cooldown_ms < 0 ||
        *count_in_window < 0)
        return -1;
    if (now_ms >= *window_start_ms + (long)window_ms) {
        *window_start_ms = now_ms;
        *count_in_window = 0;
    }
    int             decision;
    AdmissionReason reason;
    if (maintenance_mode && !emergency_override) {
        decision = 0;
        reason   = REASON_MAINTENANCE;
    } else if (now_ms < *cooldown_until_ms) {
        decision = 0;
        reason   = REASON_COOLDOWN;
    } else if (*last_accept_ms >= 0 && now_ms - *last_accept_ms < min_gap_ms) {
        decision = 0;
        reason   = REASON_MIN_GAP;
    } else if (*count_in_window >= max_in_window) {
        decision           = 0;
        reason             = REASON_WINDOW_LIMIT;
        *cooldown_until_ms = now_ms + cooldown_ms;
    } else {
        *last_accept_ms = now_ms;
        (*count_in_window)++;
        decision = 1;
        reason   = REASON_ACCEPTED;
    }
    *out_decision = decision;
    *out_reason   = reason;
    return 0;
}
static const char *reason_str(AdmissionReason r)
{
    if (r == REASON_ACCEPTED)     return "ACCEPTED";
    if (r == REASON_MAINTENANCE)  return "MAINTENANCE";
    if (r == REASON_COOLDOWN)     return "COOLDOWN";
    if (r == REASON_MIN_GAP)      return "MIN_GAP";
    if (r == REASON_WINDOW_LIMIT) return "WINDOW_LIMIT";
    return "UNKNOWN";
}
int main(void)
{
    long window_start   = 1000;
    int  window_ms      = 10000;
    int  max_in_window  = 3;
    long min_gap_ms     = 500;
    long cooldown_ms    = 3000;
    long last_accept    = -1;
    long cooldown_until = 0;
    int  count          = 0;
    int  dec;
    AdmissionReason rsn;
    int passed = 0, total = 0;
    printf("%-4s %-10s %-8s %-16s %-8s %-16s %-6s\n",
           "Test", "now_ms", "Exp.Dec", "Exp.Reason", "Got.Dec", "Got.Reason", "Result");
    printf("%.78s\n", "------------------------------------------------------------------------------");
#define RUN(now, maint, ovrd, exp_dec, exp_rsn, note) \
    do { \
        int ret = decide_admission((now), &window_start, window_ms, max_in_window, \
                                   min_gap_ms, cooldown_ms, &last_accept, \
                                   &cooldown_until, &count, (maint), (ovrd), \
                                   &dec, &rsn); \
        int ok = (ret == 0) && (dec == (exp_dec)) && (rsn == (exp_rsn)); \
        printf("%-4d %-10ld %-8d %-16s %-8d %-16s %-6s  %s\n", \
               ++total, (long)(now), (exp_dec), reason_str(exp_rsn), \
               dec, reason_str(rsn), ok ? "PASS" : "FAIL", (note)); \
        if (ok) passed++; \
    } while (0)
    /* T1: first accept, count=0, last=-1 -> ACCEPTED */
    RUN( 1000, false, false, 1, REASON_ACCEPTED,     "First accept");
    /* T2: now=1200, gap=200 < 500 -> MIN_GAP (count stays 1, last stays 1000) */
    RUN( 1200, false, false, 0, REASON_MIN_GAP,      "Gap 200 < 500");
    /* T3: now=1700, gap=700 >= 500, count=1 < 3 -> ACCEPTED (count=2, last=1700) */
    RUN( 1700, false, false, 1, REASON_ACCEPTED,     "Gap 700 >= 500");
    /* T4: now=2300, gap=600 >= 500, count=2 < 3 -> ACCEPTED (count=3, last=2300) */
    RUN( 2300, false, false, 1, REASON_ACCEPTED,     "Third accept fills window");
    /* T5: now=2400, gap=100 < 500 -> MIN_GAP fires before WINDOW_LIMIT */
    RUN( 2400, false, false, 0, REASON_MIN_GAP,      "gap=100 fires before window limit");
    /* T6: now=2900, gap=600 >= 500, count=3 >= 3 -> WINDOW_LIMIT, cooldown=5900 */
    RUN( 2900, false, false, 0, REASON_WINDOW_LIMIT, "count=3 hit limit; cooldown->5900");
    /* T7: now=5800 < 5900 -> COOLDOWN */
    RUN( 5800, false, false, 0, REASON_COOLDOWN,     "In cooldown");
    /* T8: now=5900 == cooldown_until -> not in cooldown; window 1000+10000=11000, not expired;
           count=3 >= 3 -> WINDOW_LIMIT; cooldown=5900+3000=8900 */
    RUN( 5900, false, false, 0, REASON_WINDOW_LIMIT, "Cooldown expired but window still full");
    /* T9: now=11000 >= 1000+10000=11000 -> window rolls (start=11000, count=0);
           last_accept=2300, gap=11000-2300=8700 >= 500 -> ACCEPTED (count=1, last=11000) */
    RUN(11000, false, false, 1, REASON_ACCEPTED,     "Window rolls; fresh start");
    /* T10: now=12000, maint=true, ovrd=false -> MAINTENANCE */
    RUN(12000, true,  false, 0, REASON_MAINTENANCE,  "Maintenance blocks");
    /* T11: now=12550, maint=true, ovrd=true;
            gap=12550-11000=1550 >= 500; count=1 < 3 -> ACCEPTED */
    RUN(12550, true,  true,  1, REASON_ACCEPTED,     "Override bypasses maint; gap OK");
    {
        int ret = decide_admission(1000, NULL, 10000, 3, 500, 3000,
                                   &last_accept, &cooldown_until, &count,
                                   false, false, &dec, &rsn);
        int ok = (ret == -1);
        printf("%-4d %-10s %-8s %-16s %-8s %-16s %-6s  %s\n",
               ++total, "N/A", "-1", "N/A", "N/A", "N/A",
               ok ? "PASS" : "FAIL", "NULL pointer");
        if (ok) passed++;
    }
    {
        long ws = 0;
        int ret = decide_admission(1000, &ws, 0, 3, 500, 3000,
                                   &last_accept, &cooldown_until, &count,
                                   false, false, &dec, &rsn);
        int ok = (ret == -1);
        printf("%-4d %-10s %-8s %-16s %-8s %-16s %-6s  %s\n",
               ++total, "N/A", "-1", "N/A", "N/A", "N/A",
               ok ? "PASS" : "FAIL", "window_ms=0");
        if (ok) passed++;
    }
    printf("%.78s\n", "------------------------------------------------------------------------------");
    printf("Result: %d / %d tests passed.\n", passed, total);
    return (passed == total) ? 0 : 1;
}
