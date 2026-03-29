#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

int dynint_append(int **buf, int *count, int *capacity, int value)
{
    if (buf == NULL || count == NULL || capacity == NULL)  return -1;
    if (*count < 0 || *capacity < 0)                        return -1;
    if (*capacity == 0 && *buf != NULL)                     return -1;
    if (*count > *capacity)                                    return -1;

    if (*count < *capacity) {
        (*buf)[*count] = value;
        (*count)++;
        return 0;
    }

    int new_cap;
    if (*capacity == 0) {
        new_cap = 4;
    } else {
        long nc = (long)*capacity * 2;
        if (nc > INT_MAX / (int)sizeof(int))              return -3;
        new_cap = (int)nc;
    }

    if (new_cap < *count + 1)                                 return -3;

    int *tmp;
    if (*capacity == 0)
        tmp = (int *)malloc((long)new_cap * (long)sizeof(int));
    else
        tmp = (int *)realloc(*buf, (long)new_cap * (long)sizeof(int));
    if (tmp == NULL)                                            return -2;

    *buf      = tmp;
    *capacity = new_cap;
    (*buf)[*count] = value;
    (*count)++;
    return 0;
}

int main(void) {
    int *buf = NULL, count = 0, cap = 0, r;

    r = dynint_append(&buf, &count, &cap, 10);
    printf("TC1: %s | ret=%d count=%d cap=%d buf[0]=%d\n",
           (r==0&&count==1&&cap==4&&buf[0]==10)?"PASS":"FAIL",
           r, count, cap, buf[0]);

    dynint_append(&buf, &count, &cap, 20);
    dynint_append(&buf, &count, &cap, 30);
    r = dynint_append(&buf, &count, &cap, 40);
    printf("TC2: %s | ret=%d count=%d cap=%d buf={%d,%d,%d,%d}\n",
           (r==0&&count==4&&cap==4&&buf[3]==40)?"PASS":"FAIL",
           r, count, cap, buf[0], buf[1], buf[2], buf[3]);

    r = dynint_append(&buf, &count, &cap, 50);
    printf("TC3: %s | ret=%d count=%d cap=%d buf[4]=%d\n",
           (r==0&&count==5&&cap==8&&buf[4]==50)?"PASS":"FAIL",
           r, count, cap, buf[4]);

    int bad_count = -1, bad_cap = 4;
    r = dynint_append(&buf, &bad_count, &bad_cap, 99);
    printf("TC4: %s | ret=%d\n", (r==-1)?"PASS":"FAIL", r);

    int big = INT_MAX / (int)sizeof(int) + 1;
    int ov_count = big, ov_cap = big;
    r = dynint_append(&buf, &ov_count, &ov_cap, 1);
    printf("TC5: %s | ret=%d\n", (r==-3)?"PASS":"FAIL", r);

    r = dynint_append(NULL, &count, &cap, 1);
    printf("TC6: %s | ret=%d\n", (r==-1)?"PASS":"FAIL", r);

    free(buf);
    return 0;
}