#include <stdio.h>
#include <stdbool.h>
#include <stddef.h>

int debounce_and_fire(
    const int *samples, int n, int k,
    void (*on_press)(int event_code),
    bool *out_invoked)
{
    if (samples == NULL || on_press == NULL || out_invoked == NULL ||
        n < 0 || k < 1)
        return -1;
    if (n < k) {
        *out_invoked = false;
        return 0;
    }
    bool all_one = true;
    for (int i = n - k; i < n; i++) {
        if (samples[i] != 1) { all_one = false; break; }
    }
    if (all_one) { on_press(1); *out_invoked = true; }
    else          *out_invoked = false;
    return 0;
}

static int cb_count = 0;
static void mock_cb(int ev) { cb_count++; printf("  [callback fired: event_code=%d]\n", ev); }

int main(void) {
    bool inv; int r;

    int s1[] = {0,1,1,1}; cb_count=0;
    r = debounce_and_fire(s1, 4, 3, mock_cb, &inv);
    printf("TC1: %s | ret=%d invoked=%d cb_count=%d\n",
           (r==0&&inv==true&&cb_count==1)?"PASS":"FAIL", r, inv, cb_count);

    int s2[] = {1,1,0,1}; cb_count=0;
    r = debounce_and_fire(s2, 4, 2, mock_cb, &inv);
    printf("TC2: %s | ret=%d invoked=%d cb_count=%d\n",
           (r==0&&inv==false&&cb_count==0)?"PASS":"FAIL", r, inv, cb_count);

    int s3[] = {1,1}; cb_count=0;
    r = debounce_and_fire(s3, 2, 3, mock_cb, &inv);
    printf("TC3: %s | ret=%d invoked=%d cb_count=%d\n",
           (r==0&&inv==false&&cb_count==0)?"PASS":"FAIL", r, inv, cb_count);

    int s4[] = {1,1,1,1}; cb_count=0;
    r = debounce_and_fire(s4, 4, 4, mock_cb, &inv);
    printf("TC4: %s | ret=%d invoked=%d cb_count=%d\n",
           (r==0&&inv==true&&cb_count==1)?"PASS":"FAIL", r, inv, cb_count);

    int s5[] = {0,0,0}; cb_count=0;
    r = debounce_and_fire(s5, 3, 1, mock_cb, &inv);
    printf("TC5: %s | ret=%d invoked=%d cb_count=%d\n",
           (r==0&&inv==false&&cb_count==0)?"PASS":"FAIL", r, inv, cb_count);

    r = debounce_and_fire(NULL, 4, 2, mock_cb, &inv);
    printf("TC6: %s | ret=%d\n", (r==-1)?"PASS":"FAIL", r);

    r = debounce_and_fire(s1, -1, 2, mock_cb, &inv);
    printf("TC7: %s | ret=%d\n", (r==-1)?"PASS":"FAIL", r);

    r = debounce_and_fire(s1, 4, 0, mock_cb, &inv);
    printf("TC8: %s | ret=%d\n", (r==-1)?"PASS":"FAIL", r);

    return 0;
}
