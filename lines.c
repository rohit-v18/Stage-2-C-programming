#include <stdio.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>

static int classify_line(const char *line, bool ign_blank, bool ign_comment)
{
    const char *p = line;
    while (*p == ' ' || *p == '\t') p++;
    if (ign_blank && *p == '\0')  return 0;
    if (ign_comment && *p == '#') return 0;
    return 1;
}

int count_lines_filtered(
    const char *path, bool ignore_blank,
    bool ignore_comment_hash, int *out_count)
{
    if (path == NULL || out_count == NULL) return -1;

    FILE *f = fopen(path, "rb");
    if (f == NULL) return -1;

    char chunk[4096];
    char line[8192];
    int  line_len = 0;
    int  count   = 0;
    bool in_line  = false;

    int n;
    while ((n = (int)fread(chunk, 1, sizeof(chunk), f)) > 0) {
        for (int i = 0; i < n; i++) {
            char c = chunk[i];
            if (c == '\n') {
                if (line_len > 0 && line[line_len - 1] == '\r')
                    line_len--;
                line[line_len] = '\0';
                count += classify_line(line, ignore_blank, ignore_comment_hash);
                line_len = 0;
                in_line = false;
            } else {
                if (line_len < (int)sizeof(line) - 1)
                    line[line_len++] = c;
                in_line = true;
            }
        }
    }

    if (in_line) {
        line[line_len] = '\0';
        count += classify_line(line, ignore_blank, ignore_comment_hash);
    }

    if (ferror(f)) { fclose(f); return -1; }
    fclose(f);
    *out_count = count;
    return 0;
}

static void write_file(const char *path, const char *data, int len) {
    FILE *f = fopen(path, "wb");
    if (f) { fwrite(data, 1, len, f); fclose(f); }
}

int main(void) {
    int cnt, r;

    write_file("tc1.txt", "A\nB\nC\n", 6);
    r = count_lines_filtered("tc1.txt", false, false, &cnt);
    printf("TC1: %s | ret=%d count=%d (expect 3)\n",
           (r==0&&cnt==3)?"PASS":"FAIL", r, cnt);

    write_file("tc2.txt", "A\r\n\r\n#x\r\nB\r\n", 14);
    r = count_lines_filtered("tc2.txt", true, true, &cnt);
    printf("TC2: %s | ret=%d count=%d (expect 2)\n",
           (r==0&&cnt==2)?"PASS":"FAIL", r, cnt);

    write_file("tc3.txt", " # comment\n data \n", 19);
    r = count_lines_filtered("tc3.txt", false, true, &cnt);
    printf("TC3: %s | ret=%d count=%d (expect 1)\n",
           (r==0&&cnt==1)?"PASS":"FAIL", r, cnt);

    r = count_lines_filtered("no_such_file.txt", false, false, &cnt);
    printf("TC4: %s | ret=%d (expect -1)\n",
           (r==-1)?"PASS":"FAIL", r);

    r = count_lines_filtered("tc1.txt", false, false, NULL);
    printf("TC5: %s | ret=%d (expect -1)\n",
           (r==-1)?"PASS":"FAIL", r);

    return 0;
}