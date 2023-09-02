#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

typedef struct _node {
    struct _node * prev;
    struct _node * next;
} node_t;

void insert_node(node_t *prev, node_t *cur, node_t *next) {
    if (prev != NULL) {
        prev->next = cur;
        cur->prev = prev;
    }
    if (next != NULL) {
        next->prev = cur;
        cur->next = next;
    }
}

typedef struct _tok {
    node_t node;
    char *s;
    size_t ns;
    int kind;
} tok;

#define TOK_LPAREN 0
#define TOK_RPAREN 1
#define TOK_ARROW 2
#define TOK_EQ 3
#define TOK_NEQ 4
#define TOK_LT 5
#define TOK_GT 6
#define TOK_LE 7
#define TOK_GE 8
#define TOK_PLUS 9
#define TOK_MINUS 10
#define TOK_MUL 11
#define TOK_DIV 12
#define TOK_MOD 13
#define TOK_AND 14
#define TOK_OR 15
#define TOK_NOT 16
#define TOK_DOT 17
#define TOK_COMMA 18
#define TOK_COLON 19
#define TOK_SEMICOLON 20
#define TOK_LBRACE 21
#define TOK_RBRACE 22
#define TOK_LBRACKET 23
#define TOK_RBRACKET 24
#define TOK_ID 25
#define TOK_INT 26
#define TOK_FLOAT 27
#define TOK_STRING 28
#define TOK_TRUE 29
#define TOK_FALSE 30
#define TOK_NULL 31

#define TRIE_ROOT -1

typedef struct _trie_t {
    struct _trie_t *children;
    size_t num_children;
    char c;
} trie_t;

void insert_trie(trie_t *t, char *s) {
    if (!*s) return;
    bool done = false;
    for (int i = 0; i < t->num_children; ++i) {
        if (*s == t->children[i].c) {
            insert_trie(&(t->children[i]), s+1);
            done = true;
            break;
        }
    }
    if (!done) {
        size_t depth = strlen(s);
        trie_t * new_children = (trie_t *)malloc(sizeof(trie_t) * depth);
        for (int i = 0; i < depth; ++i) {
            new_children[i].c = s[i];
            new_children[i].children = i + 1 == depth ? NULL : &(new_children[i+1]);
            new_children[i].num_children = i + 1 == depth ? 0 : 1;
        }
        trie_t * tmp = (trie_t *)malloc(sizeof(trie_t) * (t->num_children + 1));
        memcpy(tmp, t->children, sizeof(trie_t) * t->num_children);
        tmp[t->num_children] = new_children[0];
        
    }

}


tok * make_tok(char *from, char *to) {
    tok *t = (tok *)malloc(sizeof(tok));
    t->ns = to - from;
    t->s = (char *)calloc(t->ns, sizeof(char));
    strncpy(t->s, from, t->ns);
    t->kind = kindof(t);
    return t;
}

tok * tokenize(char *s) {
    char *slow, *fast;
    char iswhite[256] = 0;
    char whites = " \n\r\t";
    for (char *p = whites; *p; ++p) {
        iswhite[*p] = 1;
    }
    size_t ns = strlen(s);
    for (slow = fast = s; fast < s + ns; ) {
        if (iswhite[*fast]) {
            tok *t = make_tok(slow, fast);

        } else {
            continue;
        }
    }
    // shake of the remainder in [slow, fast) buffer once again
}

typedef struct _stmt {
    node_t node;
    char *s;
    size_t ns;
} stmt;

stmt * parse_stmts(char *s) {
}