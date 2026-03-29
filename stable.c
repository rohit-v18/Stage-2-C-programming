#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>

static int abs_diff(int x, int y) { return (x > y) ? (x-y) : (y-x); }

int find_longest_stable_segment(
    const int *a, int n, int step_max, int span_max,
    double avg_min, double avg_max, int min_len, int max_len,
    int *out_start, int *out_end, int *out_len, double *out_avg)
{
    if (a==NULL||out_start==NULL||out_end==NULL||out_len==NULL||out_avg==NULL) return -1;
    if (n<0||step_max<0||span_max<0||avg_min>avg_max||min_len<1||(max_len>0&&max_len<min_len)) return -1;
    if (n==0) return -2;
    int best_L=-1, best_R=-1, best_len=0;
    double best_avg=0.0;
    for (int L=0; L<n; L++) {
        long sum=a[L]; int vmin=a[L], vmax=a[L];
        for (int R=L; R<n; R++) {
            if (R>L) {
                if (abs_diff(a[R],a[R-1])>step_max) break;
                sum+=a[R];
                if(a[R]<vmin) vmin=a[R];
                if(a[R]>vmax) vmax=a[R];
            }
            int seg_len=R-L+1;
            if(max_len>0&&seg_len>max_len) break;
            if(seg_len>=min_len&&(vmax-vmin)<=span_max) {
                double mean=(double)sum/(double)seg_len;
                if(mean>=avg_min&&mean<=avg_max) {
                    if(seg_len>best_len||(seg_len==best_len&&mean>best_avg)||
                       (seg_len==best_len&&mean==best_avg&&L<best_L)) {
                        best_L=L; best_R=R; best_len=seg_len; best_avg=mean;
                    }
                }
            }
        }
    }
    if(best_len==0) return -2;
    *out_start=best_L; *out_end=best_R; *out_len=best_len; *out_avg=best_avg;
    return 0;
}

static void check(int tc, int ret, int s, int e, int l, double avg,
                   int exp_ret, int exp_s, int exp_e, int exp_l, double exp_avg) {
    int pass;
    if (exp_ret != 0) pass = (ret == exp_ret);
    else pass = (ret==0 && s==exp_s && e==exp_e && l==exp_l);
    printf("TC%d: %s | ret=%d s=%d e=%d len=%d avg=%.2f\n",
           tc, pass?"PASS":"FAIL", ret, s, e, l, avg);
    (void)exp_avg;
}

int main(void) {
    int s, e, l; double av; int r;

    int a1[]={10,12,13,13,15,20};
    r=find_longest_stable_segment(a1,6,3,6,11.0,15.0,3,0,&s,&e,&l,&av);
    check(1,r,s,e,l,av, 0,0,4,5,12.6);

    int a2[]={5,5,5,5};
    r=find_longest_stable_segment(a2,4,0,0,5.0,5.0,2,0,&s,&e,&l,&av);
    check(2,r,s,e,l,av, 0,0,3,4,5.0);

    int a3[]={1,20,2,21,3,22};
    r=find_longest_stable_segment(a3,6,3,5,0.0,100.0,2,0,&s,&e,&l,&av);
    check(3,r,s,e,l,av, 0,4,5,2,12.5);

    int a4[]={3,4,7,8,9};
    r=find_longest_stable_segment(a4,5,4,3,5.0,7.0,2,3,&s,&e,&l,&av);
    check(4,r,s,e,l,av, 0,2,3,2,7.5);

    int a5[]={100};
    r=find_longest_stable_segment(a5,1,0,0,200.0,300.0,1,0,&s,&e,&l,&av);
    check(5,r,s,e,l,av, -2,0,0,0,0);

    int a6[]={-2,-1,-1,0,1};
    r=find_longest_stable_segment(a6,5,2,3,-1.5,0.5,3,0,&s,&e,&l,&av);
    check(6,r,s,e,l,av, 0,0,4,5,-0.6);

    r=find_longest_stable_segment(NULL,5,1,1,0.0,1.0,1,0,&s,&e,&l,&av);
    check(7,r,s,e,l,av, -1,0,0,0,0);

    int a8[]={1,2};
    r=find_longest_stable_segment(a8,2,1,1,2.0,1.0,1,0,&s,&e,&l,&av);
    check(8,r,s,e,l,av, -1,0,0,0,0);

    return 0;
}