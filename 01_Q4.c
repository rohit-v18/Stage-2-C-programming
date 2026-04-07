#include <stdio.h>
#include <stdbool.h>
typedef enum { ST_LOCKED, ST_UNLOCKED, ST_MOVING_LOCK,
               ST_MOVING_UNLOCK, ST_ALARM, ST_LOWPOWER } LockState;
typedef enum { EV_LOCK_CMD, EV_UNLOCK_CMD, EV_MOTOR_DONE_OK,
               EV_MOTOR_STALL, EV_FORCED_OPEN, EV_TIMEOUT,
               EV_BATTERY_LOW, EV_BATTERY_OK } LockEvent;
typedef enum { ACT_NONE, ACT_START_LOCK, ACT_START_UNLOCK,
               ACT_SOUND_ALARM, ACT_ENTER_SLEEP } Action;
int lock_step(LockState curr, LockEvent ev, bool battery_low, int attempts,
              LockState *out_next, Action *out_action, int *out_attempts_next)
{
    if (!out_next || !out_action || !out_attempts_next) return -1;
    if (curr < ST_LOCKED   || curr > ST_LOWPOWER)      return -1;
    if (ev   < EV_LOCK_CMD || ev   > EV_BATTERY_OK)    return -1;
    if (attempts < 0)                                   return -1;
    LockState next  = curr;
    Action    act   = ACT_NONE;
    int       att   = attempts;
    if (battery_low && ev == EV_BATTERY_LOW && curr != ST_ALARM) {
        next = ST_LOWPOWER;
        act  = ACT_ENTER_SLEEP;
        goto done;
    }
    if (curr == ST_LOWPOWER && ev == EV_BATTERY_OK) {
        next = ST_UNLOCKED;
        act  = ACT_NONE;
        att  = 0;
        goto done;
    }
    if (ev == EV_FORCED_OPEN) {
        next = ST_ALARM;
        act  = ACT_SOUND_ALARM;
        att  = 0;
        goto done;
    }
    if (ev == EV_LOCK_CMD || ev == EV_UNLOCK_CMD) {
        if (attempts >= 3) {
            goto done;
        }
        if (ev == EV_LOCK_CMD && curr == ST_UNLOCKED) {
            next = ST_MOVING_LOCK;
            act  = ACT_START_LOCK;
            att  = attempts + 1;
        } else if (ev == EV_UNLOCK_CMD && curr == ST_LOCKED) {
            next = ST_MOVING_UNLOCK;
            act  = ACT_START_UNLOCK;
            att  = attempts + 1;
        }
        goto done;
    }
    if (curr == ST_MOVING_LOCK || curr == ST_MOVING_UNLOCK) {
        if (ev == EV_MOTOR_DONE_OK) {
            next = (curr == ST_MOVING_LOCK) ? ST_LOCKED : ST_UNLOCKED;
            act  = ACT_NONE;
            att  = 0;
        } else if (ev == EV_MOTOR_STALL || ev == EV_TIMEOUT) {
            next = ST_ALARM;
            act  = ACT_SOUND_ALARM;
            att  = 0;
        }
        goto done;
    }
done:
    *out_next          = next;
    *out_action        = act;
    *out_attempts_next = att;
    return 0;
}
int main(void)
{
    struct {
        LockState curr;
        LockEvent ev;
        bool      battery_low;
        int       attempts;
        int       exp_ret;
        LockState exp_next;
        Action    exp_act;
        int       exp_att;
        const char *note;
    } tests[] = {
        { ST_UNLOCKED,     EV_LOCK_CMD,     false, 0,  0, ST_MOVING_LOCK,   ACT_START_LOCK,   1, "Normal lock command"            },
        { ST_MOVING_LOCK,  EV_MOTOR_DONE_OK,false, 1,  0, ST_LOCKED,        ACT_NONE,         0, "Motor completes lock"           },
        { ST_LOCKED,       EV_UNLOCK_CMD,   false, 3,  0, ST_LOCKED,        ACT_NONE,         3, "Command ignored (attempt limit)" },
        { ST_MOVING_UNLOCK,EV_TIMEOUT,      false, 1,  0, ST_ALARM,         ACT_SOUND_ALARM,  0, "Timeout while moving"           },
        { ST_LOCKED,       EV_FORCED_OPEN,  false, 0,  0, ST_ALARM,         ACT_SOUND_ALARM,  0, "Forced open security breach"    },
        { ST_UNLOCKED,     EV_BATTERY_LOW,  true,  0,  0, ST_LOWPOWER,      ACT_ENTER_SLEEP,  0, "Enter low-power mode"          },
        { ST_LOWPOWER,     EV_BATTERY_OK,   false, 0,  0, ST_UNLOCKED,      ACT_NONE,         0, "Recover from low-power"        },
        { (LockState)99,   EV_LOCK_CMD,     false, 0, -1, 0,                0,                0, "Invalid enum argument"         },
    };
    int n = (int)(sizeof tests / sizeof tests[0]);
    int passed = 0;
    printf("%-4s %-34s %-8s %-8s %-6s\n", "Test", "Note", "Exp.Ret", "Got.Ret", "Result");
    printf("%.65s\n", "-----------------------------------------------------------------");
    for (int i = 0; i < n; i++) {
        LockState next = (LockState)0;
        Action    act  = ACT_NONE;
        int       att  = -1;
        int ret = lock_step(tests[i].curr, tests[i].ev, tests[i].battery_low,
                            tests[i].attempts, &next, &act, &att);
        int ok = (ret == tests[i].exp_ret);
        if (ok && ret == 0)
            ok = (next == tests[i].exp_next) &&
                 (act  == tests[i].exp_act)  &&
                 (att  == tests[i].exp_att);
        printf("%-4d %-34s %-8d %-8d %-6s\n",
               i + 1, tests[i].note, tests[i].exp_ret, ret,
               ok ? "PASS" : "FAIL");
        if (ok) passed++;
    }
    printf("%.65s\n", "-----------------------------------------------------------------");
    printf("Result: %d / %d tests passed.\n", passed, n);
    return (passed == n) ? 0 : 1;
}
