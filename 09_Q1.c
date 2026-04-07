#include <stdio.h>
#include <stdbool.h>
#include <stddef.h>

enum Mode { MODE0=0, MODE1=1, MODE2=2, MODE3=3 };

struct StatusBits {
    int rsv     : 7;
    int fault   : 1;
    int busy    : 1;
    int ready   : 1;
    int mode    : 2;
    int retries : 3;
    int par     : 1;
};

int build_status_reg16(
    enum Mode mode, int retries,
    bool ready, bool busy, bool fault,
    int *out_reg16)
{
    if (out_reg16 == NULL)                          return -1;
    if (retries < 0 || retries > 7)               return -1;
    if (mode < MODE0 || mode > MODE3)              return -1;

    int reg = 0;
    reg |= (retries       & 0x7) << 12;
    reg |= ((int)mode     & 0x3) << 10;
    reg |= (ready  ? 1 : 0)      <<  9;
    reg |= (busy   ? 1 : 0)      <<  8;
    reg |= (fault  ? 1 : 0)      <<  7;

    int p = 0, tmp = reg & 0x7FFF;
    while (tmp) { p ^= (tmp & 1); tmp >>= 1; }
    reg |= (p & 1) << 15;

    *out_reg16 = reg;
    return 0;
}

static void print_reg(int reg) {
    printf("  PAR=%d RETRIES=%d MODE=%d READY=%d BUSY=%d FAULT=%d RSV=%d (0x%04X)\n",
           (reg >> 15) & 1,
           (reg >> 12) & 7,
           (reg >> 10) & 3,
           (reg >>  9) & 1,
           (reg >>  8) & 1,
           (reg >>  7) & 1,
            reg         & 0x7F, reg & 0xFFFF);
}

static int even_parity_ok(int reg) {
    int cnt = 0, v = reg & 0xFFFF;
    while (v) { cnt += v & 1; v >>= 1; }
    return (cnt % 2) == 0;
}

int main(void) {
    int reg, r;

    r = build_status_reg16(MODE0, 0, false, false, false, &reg);
    printf("TC1: %s | ret=%d parity_ok=%d\n",
           (r==0 && even_parity_ok(reg))?"PASS":"FAIL", r, even_parity_ok(reg));
    print_reg(reg);

    r = build_status_reg16(MODE3, 7, true, true, true, &reg);
    printf("TC2: %s | ret=%d parity_ok=%d\n",
           (r==0 && ((reg>>12)&7)==7 && ((reg>>10)&3)==3 &&
            ((reg>>9)&1)==1 && ((reg>>8)&1)==1 && ((reg>>7)&1)==1 &&
            even_parity_ok(reg))?"PASS":"FAIL", r, even_parity_ok(reg));
    print_reg(reg);

    r = build_status_reg16(MODE1, 5, true, false, false, &reg);
    printf("TC3: %s | ret=%d parity_ok=%d\n",
           (r==0 && ((reg>>12)&7)==5 && ((reg>>10)&3)==1 &&
            ((reg>>9)&1)==1 && even_parity_ok(reg))?"PASS":"FAIL", r, even_parity_ok(reg));
    print_reg(reg);

    r = build_status_reg16(MODE0, -1, false, false, false, &reg);
    printf("TC4: %s | ret=%d\n", (r==-1)?"PASS":"FAIL", r);

    r = build_status_reg16((enum Mode)4, 3, false, false, false, &reg);
    printf("TC5: %s | ret=%d\n", (r==-1)?"PASS":"FAIL", r);

    r = build_status_reg16(MODE0, 3, false, false, false, NULL);
    printf("TC6: %s | ret=%d\n", (r==-1)?"PASS":"FAIL", r);

    return 0;
}
