#include <stdio.h>
#include <stdbool.h>
#include <stddef.h>

// Return code type
typedef enum {
    RC_BAD_ARG   = -1,
    RC_NOFAULT   =  0,
    RC_SEEN_ONLY =  1,
    RC_CLEARED   =  2
} RetCode;

// Define the global tick here (was extern-only before, causing linker error)
long g_ms_tick = 0;

RetCode handle_w1c_fault(volatile int *reg,
                         int           fault_bit,
                         bool          allow_clear,
                         long          cooldown_ms,
                         int          *out_seen_count)
{
    if (reg == NULL || out_seen_count == NULL) return RC_BAD_ARG;
    if (fault_bit < 0 || fault_bit > 30)       return RC_BAD_ARG;
    if (cooldown_ms < 0)                        return RC_BAD_ARG;

    static int  seen_count    = 0;
    static long last_clear_ms = 0;

    int mask = (1 << fault_bit);
    int val  = *reg;

    if (!(val & mask)) {
        *out_seen_count = seen_count;
        return RC_NOFAULT;
    }

    ++seen_count;

    if (allow_clear && (g_ms_tick - last_clear_ms >= cooldown_ms)) {
        *reg            = mask;
        last_clear_ms   = g_ms_tick;
        *out_seen_count = seen_count;
        return RC_CLEARED;
    }

    *out_seen_count = seen_count;
    return RC_SEEN_ONLY;
}

static const char *rc_name(RetCode rc) {
    switch (rc) {
        case RC_NOFAULT:   return "RC_NOFAULT";
        case RC_SEEN_ONLY: return "RC_SEEN_ONLY";
        case RC_CLEARED:   return "RC_CLEARED";
        case RC_BAD_ARG:   return "RC_BAD_ARG";
        default:           return "UNKNOWN";
    }
}

int main(void) {
    volatile int reg;
    int seen = 0;
    RetCode rc;

    // Test 1: no fault
    reg = 0x00; g_ms_tick = 0;
    rc = handle_w1c_fault(&reg, 3, true, 0, &seen);
    printf("T1: %s, seen=%d, reg=0x%02X\n", rc_name(rc), seen, reg);

    // Test 2: fault set, clear not allowed
    reg = 0x08;
    rc = handle_w1c_fault(&reg, 3, false, 0, &seen);
    printf("T2: %s, seen=%d, reg=0x%02X\n", rc_name(rc), seen, reg);

    // Test 3: fault set, clear allowed, cooldown=0
    reg = 0x08; g_ms_tick = 100;
    rc = handle_w1c_fault(&reg, 3, true, 0, &seen);
    printf("T3: %s, seen=%d, reg=0x%02X\n", rc_name(rc), seen, reg);

    // Test 4: fault again, cooldown=50, only 20ms elapsed
    reg = 0x08; g_ms_tick = 120;
    rc = handle_w1c_fault(&reg, 3, true, 50, &seen);
    printf("T4: %s, seen=%d, reg=0x%02X\n", rc_name(rc), seen, reg);

    // Test 5: fault again, 80ms elapsed (>= 50ms cooldown)
    reg = 0x08; g_ms_tick = 180;
    rc = handle_w1c_fault(&reg, 3, true, 50, &seen);
    printf("T5: %s, seen=%d, reg=0x%02X\n", rc_name(rc), seen, reg);

    // Test 6: invalid args
    rc = handle_w1c_fault(NULL, 3, true, 0, &seen);
    printf("T6a: %s\n", rc_name(rc));
    rc = handle_w1c_fault(&reg, 31, true, 0, &seen);
    printf("T6b: %s\n", rc_name(rc));
    rc = handle_w1c_fault(&reg, 3, true, 0, NULL);
    printf("T6c: %s\n", rc_name(rc));

    return 0;
}
