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

struct xmlattr {
    char *name;
    char *value;
};
typedef struct xmlattr xmlattr;

struct xmltag {
    char *name;

    /* attributes */
    int attrn;
    xmlattr *attrs;

    struct xmltag *parent;
    /* children */
    int childn;
    struct xmltag **children;
};
typedef struct xmltag xmltag;

void xmlAddChild(xmltag *parent, xmltag *child) {
    if (!parent) return;
    if (!child) return;

    child->parent = parent;

    parent->childn++;
    parent->children = realloc(parent->children, parent->childn * sizeof(xmltag*));
    parent->children[parent->childn - 1] = child;
}

/* may change string "attr" */
void xmlAddAttr(xmltag *tag, char *str) {
    if (!tag) return;
    if (!str) return;

    tag->attrn++;
    tag->attrs = realloc(tag->attrs, tag->attrn * sizeof(tag->attrs[0]));

    xmlattr *attr = &tag->attrs[tag->attrn - 1];

    attr->name  = str;
    attr->value = 0;

    int i;
    for (i = 0; str[i]; ++i) {
        if (str[i] == '=') {
            str[i] = 0;
        }
        if (str[i] == '"') {
            if (!attr->value) attr->value = &str[i + 1];
            str[i] = 0;
        }
    }
}

void xmlDel(xmltag *tag) {
    if (!tag) return;
    free(tag->name); /* also frees attribute values */
    free(tag->attrs);

    int i;
    for (i = 0; i < tag->childn; ++i) free(tag->children[i]);

    free(tag->children);
    free(tag);
}

xmltag *xmlGetTag(FILE *file) {
    /* get opening row */
    char *open = xmlRow(file);
    char **opena = strSplit(open, " \n\t\r");
    if (!opena || !opena[0]) {
        free(open);
        return 0;
    }

    char *name = opena[0];

    xmltag *tag = calloc(1, sizeof(xmltag));
    tag->name = name;

    char closingtag = 1; /* is there a closing tag? */

    if (tag->name[0] == '/') closingtag = 0;
    /* information and comment tags */
    if (name[0] == '?') closingtag = 0;
    if (name[0] == '!' && name[1] == '-' && name[2] == '-') closingtag = 0;

    /* fetch attributes */
    int i = 1;
    while (opena[i]) {
        char *attr = opena[i];
        if (!strcmp(attr, "/")) {
            /* there is no closing tag */
            closingtag = 0;
        } else {
            xmlAddAttr(tag, attr);
        }
        ++i;
    }

    if (!closingtag) return tag;

    while (1) {
        xmltag *child = xmlGetTag(file);
        if (!child) break;
        if (child->name[0] == '/' && !strcmp(&child->name[1], tag->name)) {
            /* closing tag found */
            xmlDel(child);
            break;
        }

        xmlAddChild(tag, child);
    }

    return tag;
}

/* print xml structure recursively */
void xmlPrint(xmltag *tag, int level) {
    if (!tag) return;

    /* print open row */
    int i;
    for (i = 0; i < level; ++i) printf("  ");
    printf("<%s", tag->name);
    for (i = 0; i < tag->attrn; ++i) {
        char *name  = tag->attrs[i].name;
        char *value = tag->attrs[i].value;
        printf(" %s", name);
        if (value) printf("=\"%s\"", value);
    }
    printf(">\n");

    /* print children */
    for (i = 0; i < tag->childn; ++i) xmlPrint(tag->children[i], level + 1);

    /* print close row */
    for (i = 0; i < level; ++i) printf("  ");
    printf("</%s>\n", tag->name);
}

int main() {
    FILE *file = fopen("xml.xml", "r");
    xmltag *tag;
    while (1) {
        tag = xmlGetTag(file);
        xmlPrint(tag, 0);
        if (!tag) break;
        xmlDel(tag);
    }
    fclose(file);
    return 0;
}
