#include <stdio.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>

int safe_copy_label(const char *src, char *dst, int dst_capacity,
                    bool to_lower, int *out_copied, bool *out_truncated);

int main(void) {
    char buf[64];
    int copied; bool trunc; int r;

    memset(buf, 0xAB, sizeof(buf));
    r = safe_copy_label("Hello", buf, 10, false, &copied, &trunc);
    printf("TC1: %s | ret=%d copied=%d trunc=%d dst=\"%s\"\n",
           (r==0&&copied==5&&!trunc&&strcmp(buf,"Hello")==0)?"PASS":"FAIL",
           r, copied, trunc, buf);

    memset(buf, 0xAB, sizeof(buf));
    r = safe_copy_label("Hello", buf, 6, true, &copied, &trunc);
    printf("TC2: %s | ret=%d copied=%d trunc=%d dst=\"%s\"\n",
           (r==0&&copied==5&&!trunc&&strcmp(buf,"hello")==0)?"PASS":"FAIL",
           r, copied, trunc, buf);

    memset(buf, 0xAB, sizeof(buf));
    r = safe_copy_label("FirmwareV1", buf, 5, false, &copied, &trunc);
    printf("TC3: %s | ret=%d copied=%d trunc=%d dst=\"%s\"\n",
           (r==0&&copied==4&&trunc&&strcmp(buf,"Firm")==0)?"PASS":"FAIL",
           r, copied, trunc, buf);

    memset(buf, 0xAB, sizeof(buf));
    r = safe_copy_label("", buf, 3, true, &copied, &trunc);
    printf("TC4: %s | ret=%d copied=%d trunc=%d dst=\"%s\"\n",
           (r==0&&copied==0&&!trunc&&strcmp(buf,"")==0)?"PASS":"FAIL",
           r, copied, trunc, buf);

    r = safe_copy_label("ABC", buf, 0, false, &copied, &trunc);
    printf("TC5: %s | ret=%d copied=%d trunc=%d\n",
           (r==0&&copied==0&&trunc)?"PASS":"FAIL", r, copied, trunc);

    r = safe_copy_label(NULL, buf, 10, false, &copied, &trunc);
    printf("TC6: %s | ret=%d\n", (r==-1)?"PASS":"FAIL", r);

    r = safe_copy_label("ABC", buf, -1, false, &copied, &trunc);
    printf("TC7: %s | ret=%d\n", (r==-1)?"PASS":"FAIL", r);

    return 0;
}
