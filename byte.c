#include <stdio.h>
#include <stddef.h>

int find_subsequence_in_slice(
    const char *buf, int n,
    const char *needle, int m,
    char **out_ptr, int *out_offset)
{
    if (buf == NULL || needle == NULL || out_ptr == NULL ||
        out_offset == NULL || n < 0 || m <= 0)
        return -1;
    const char *end = buf + n;
    for (const char *p = buf; p + m <= end; ++p) {
        const char *q = p;
        const char *r = needle;
        while (r < needle + m && *q == *r) { ++q; ++r; }
        if (r == needle + m) {
            *out_ptr    = (char *)p;
            *out_offset = (int)(p - buf);
            return 0;
        }
    }
    *out_ptr    = NULL;
    *out_offset = -1;
    return 0;
}

int main(void) {
    char *ptr; int off, r;

    const char *b1 = "ABCDEF";
    r = find_subsequence_in_slice(b1, 6, "CDE", 3, &ptr, &off);
    printf("TC1: %s | ret=%d off=%d\n",
           (r==0 && off==2 && ptr==b1+2) ? "PASS" : "FAIL", r, off);

    const char *b2 = "AAAAA";
    r = find_subsequence_in_slice(b2, 5, "AAA", 3, &ptr, &off);
    printf("TC2: %s | ret=%d off=%d\n",
           (r==0 && off==0 && ptr==b2) ? "PASS" : "FAIL", r, off);

    const char *b3 = "ABCDE";
    r = find_subsequence_in_slice(b3, 5, "DEF", 3, &ptr, &off);
    printf("TC3: %s | ret=%d off=%d ptr=%s\n",
           (r==0 && off==-1 && ptr==NULL) ? "PASS" : "FAIL", r, off,
           ptr ? ptr : "NULL");

    const char *b4 = "XYZ";
    r = find_subsequence_in_slice(b4, 3, "XYZ", 3, &ptr, &off);
    printf("TC4: %s | ret=%d off=%d\n",
           (r==0 && off==0 && ptr==b4) ? "PASS" : "FAIL", r, off);

    const char *b5 = "HI";
    r = find_subsequence_in_slice(b5, 2, "HIK", 3, &ptr, &off);
    printf("TC5: %s | ret=%d off=%d ptr=%s\n",
           (r==0 && off==-1 && ptr==NULL) ? "PASS" : "FAIL", r, off,
           ptr ? ptr : "NULL");

    r = find_subsequence_in_slice(NULL, 5, "AB", 2, &ptr, &off);
    printf("TC6: %s | ret=%d\n", (r==-1) ? "PASS" : "FAIL", r);

    r = find_subsequence_in_slice(b1, -1, "AB", 2, &ptr, &off);
    printf("TC7: %s | ret=%d\n", (r==-1) ? "PASS" : "FAIL", r);

    r = find_subsequence_in_slice(b1, 6, "AB", 0, &ptr, &off);
    printf("TC8: %s | ret=%d\n", (r==-1) ? "PASS" : "FAIL", r);

    return 0;
}