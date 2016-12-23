#include <stdlib.h>
#include <stdio.h>

/* get string between next "<" and ">" */
/* returns 0 if no such string found */
char *xmlRow(FILE *file) {
    if (!file) return;

    char c;
    char inside = 0; /* inside row? */

    int len = 0;
    char *row = 0;

    while (1) {
        int res = fscanf(file, "%c", &c);
        if (res < 0) break; /* end of file */
        if (!inside) {
            if (c == '<') inside = 1;
        } else {
            if (c == '>') break;
            /* append character to row */
            len++;
            row = realloc(row, len + 1);
            row[len - 1] = c;
            row[len] = 0;
        }
    }

    return row;
}

int main() {
    FILE *file = fopen("xml.xml", "r");
    char *row;
    while (1) {
        row = xmlRow(file);
        if (!row) return;
        printf("<%s>\n", row);
        free(row);
    }
    fclose(file);
    return 0;
}
