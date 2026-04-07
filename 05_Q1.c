#include <stdio.h>
#include <stddef.h>
int find_min_in_array(const int *a, int n, int *out_min) {
    if (a == NULL || out_min == NULL || n <= 0) return -1;
    int min_value = a[0];
    for (int i = 1; i < n; i++)
        if (a[i] < min_value) min_value = a[i];
    *out_min = min_value;
    return 0;
}
int main(void) {
    int result, min;
    int a1[] = {5, 2, 8, 1, 9};
    result = find_min_in_array(a1, 5, &min);
    printf("Test 1: ret=%d, min=%d (expect ret=0, min=1)\n", result, min);
    int a2[] = {10};
    result = find_min_in_array(a2, 1, &min);
    printf("Test 2: ret=%d, min=%d (expect ret=0, min=10)\n", result, min);
    int a3[] = {-5, -2, -9, -1};
    result = find_min_in_array(a3, 4, &min);
    printf("Test 3: ret=%d, min=%d (expect ret=0, min=-9)\n", result, min);
    result = find_min_in_array(NULL, 5, &min);
    printf("Test 4: ret=%d        (expect ret=-1)\n", result);
    result = find_min_in_array(a1, 0, &min);
    printf("Test 5: ret=%d        (expect ret=-1)\n", result);
    result = find_min_in_array(a1, 5, NULL);
    printf("Test 6: ret=%d        (expect ret=-1)\n", result);
    return 0;
}
