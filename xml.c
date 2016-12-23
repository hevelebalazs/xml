#include <stdlib.h>
#include <stdio.h>

/* is character "c" in sting "set"? */
char strInset(char c, char *set) {
    int i;
    for (i = 0; set[i]; ++i) {
        if (set[i] == c) return 1;
    }
    return 0;
}

/* split string by any character in string delim */
/* the array of pointers will be terminated by a 0 */
/* modifies the string, and uses the same memory */
char **strSplit(char *str, char *delim) {
    if (!str) return 0;

    int partn = 0; /* number of parts */
    char **parts = 0;

    char isdelim = 1; /* is the character a delimiter? */

    int i;
    for (i = 0; str[i]; ++i) {
        char prevdelim = isdelim;
        char c = str[i];
        isdelim = strInset(c, delim);

        if (isdelim) str[i] = 0;
        else if (prevdelim) {
            /* add pointer to "parts" array */
            partn++;
            parts = realloc(parts, partn * sizeof(char*));
            parts[partn - 1] = &str[i];
        }
    }

    /* add 0 to "parts" array */
    partn++;
    parts = realloc(parts, partn * sizeof(char*));
    parts[partn - 1] = 0;
    return parts;
}

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

        char **attrs = strSplit(row, " \n\t\r");
        /* print */
        int i;
        printf("<\n");
        for (i = 0; attrs[i]; ++i) printf(" %s\n", attrs[i]);
        printf(">\n");

        free(row);
    }
    fclose(file);
    return 0;
}
