#include <stdio.h>
#include <stddef.h>

/* ---------------------------------------------------------------
   Insertion sort on a[left..right] — stable (swap only when strictly less)
   --------------------------------------------------------------- */
static void insertion_sort(int *a, int left, int right) {
    for (int i = left + 1; i <= right; i++) {
        int key = a[i];
        int j   = i - 1;
        while (j >= left && a[j] > key) {   /* strictly > keeps equal elements stable */
            a[j + 1] = a[j];
            j--;
        }
        a[j + 1] = key;
    }
}

/* ---------------------------------------------------------------
   Merge a[left..mid] and a[mid+1..right] into scratch, copy back.
   Left element wins on equality → stable.
   --------------------------------------------------------------- */
static void merge(int *a, int left, int mid, int right, int *scratch) {
    int i = left;       /* pointer into left half  */
    int j = mid + 1;    /* pointer into right half */
    int k = 0;          /* pointer into scratch    */

    while (i <= mid && j <= right) {
        if (a[i] <= a[j])           /* <= : left wins on equality (stable) */
            scratch[k++] = a[i++];
        else
            scratch[k++] = a[j++];
    }
    while (i <= mid)   scratch[k++] = a[i++];
    while (j <= right) scratch[k++] = a[j++];

    /* copy back */
    for (int x = 0; x < k; x++)
        a[left + x] = scratch[x];
}

/* ---------------------------------------------------------------
   Recursive top-down mergesort on a[left..right].
   scratch must have capacity >= (right - left + 1).
   --------------------------------------------------------------- */
static void mergesort(int *a, int left, int right, int *scratch) {
    int len = right - left + 1;

    if (len <= 1) return;

    /* optional small-run optimisation */
    if (len <= 16) {
        insertion_sort(a, left, right);
        return;
    }

    int mid = left + (right - left) / 2;   /* overflow-safe midpoint */

    mergesort(a, left,    mid,   scratch);
    mergesort(a, mid + 1, right, scratch);
    merge     (a, left,   mid,   right, scratch);
}

/* ---------------------------------------------------------------
   Public API
   --------------------------------------------------------------- */
int stable_sort_range_with_scratch(int *a,
                                   int  n,
                                   int  left,
                                   int  right,
                                   int *scratch,
                                   int  scratch_capacity,
                                   int *out_sorted_count)
{
    /* --- pointer validation --- */
    if (a == NULL || scratch == NULL || out_sorted_count == NULL)
        return -1;

    /* --- empty array special case --- */
    if (n == 0) {
        *out_sorted_count = 0;
        return 0;
    }

    /* --- numeric range validation --- */
    if (n < 0 || left < 0 || right < left || right >= n)
        return -1;

    int m = right - left + 1;   /* elements to sort */

    /* --- scratch capacity check --- */
    if (scratch_capacity < m)
        return -1;

    /* --- sort --- */
    mergesort(a, left, right, scratch);

    *out_sorted_count = m;
    return 0;
}

/* ===============================================================
   Test driver
   =============================================================== */
static void print_array(const char *label, int *a, int n) {
    printf("%s: [", label);
    for (int i = 0; i < n; i++)
        printf("%d%s", a[i], i < n - 1 ? ", " : "");
    printf("]\n");
}

int main(void) {
    int scratch[16];
    int out = 0;
    int ret;

    /* T1 — whole-array sort */
    int a1[] = {5, 3, 3, 2, 9};
    ret = stable_sort_range_with_scratch(a1, 5, 0, 4, scratch, 5, &out);
    printf("T1: ret=%d, out=%d ", ret, out);
    print_array("a", a1, 5);   /* expect [2,3,3,5,9] */

    /* T2 — subrange [1..3] only */
    int a2[] = {4, 1, 4, 2, 4};
    ret = stable_sort_range_with_scratch(a2, 5, 1, 3, scratch, 3, &out);
    printf("T2: ret=%d, out=%d ", ret, out);
    print_array("a", a2, 5);   /* expect [4,1,2,4,4] */

    /* T3 — stability (equal elements keep original order) */
    int a3[] = {3, 3, 3};
    ret = stable_sort_range_with_scratch(a3, 3, 0, 2, scratch, 3, &out);
    printf("T3: ret=%d, out=%d ", ret, out);
    print_array("a", a3, 3);   /* expect [3,3,3] */

    /* T4 — already sorted */
    int a4[] = {1, 2, 3, 4};
    ret = stable_sort_range_with_scratch(a4, 4, 0, 3, scratch, 4, &out);
    printf("T4: ret=%d, out=%d ", ret, out);
    print_array("a", a4, 4);   /* expect [1,2,3,4] */

    /* T5 — reverse order */
    int a5[] = {4, 3, 2, 1};
    ret = stable_sort_range_with_scratch(a5, 4, 0, 3, scratch, 4, &out);
    printf("T5: ret=%d, out=%d ", ret, out);
    print_array("a", a5, 4);   /* expect [1,2,3,4] */

    /* T6 — scratch too small */
    int a6[] = {4, 3, 2, 1};
    ret = stable_sort_range_with_scratch(a6, 4, 0, 3, scratch, 2, &out);
    printf("T6: ret=%d (expect -1)\n", ret);

    /* T7 — NULL pointers */
    ret = stable_sort_range_with_scratch(NULL, 4, 0, 3, scratch, 4, &out);
    printf("T7a: ret=%d (expect -1)\n", ret);
    ret = stable_sort_range_with_scratch(a5, 4, 0, 3, NULL, 4, &out);
    printf("T7b: ret=%d (expect -1)\n", ret);
    ret = stable_sort_range_with_scratch(a5, 4, 0, 3, scratch, 4, NULL);
    printf("T7c: ret=%d (expect -1)\n", ret);

    /* T8 — n=0 empty array */
    ret = stable_sort_range_with_scratch(a5, 0, 0, 0, scratch, 4, &out);
    printf("T8: ret=%d, out=%d (expect 0,0)\n", ret, out);

    /* T9 — invalid index range */
    int a9[] = {1, 2, 3};
    ret = stable_sort_range_with_scratch(a9, 3, 2, 1, scratch, 4, &out);  /* left > right */
    printf("T9a: ret=%d (expect -1)\n", ret);
    ret = stable_sort_range_with_scratch(a9, 3, 0, 5, scratch, 4, &out);  /* right >= n  */
    printf("T9b: ret=%d (expect -1)\n", ret);

    return 0;
}
