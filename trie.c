#include "array.h"

typedef struct _trie {
    char c;
    array* children;
} trie;
#define TRIE_ROOT -1

void trie_pprint(trie *t, int depth) {
    if (t->c == TRIE_ROOT) {
        printf("TRIE ROOT\n");
    } else {
        for(int i = 0; i < depth; ++i) {
            printf("  ");
        }
        printf("-%c\n", t->c);
    }
    for (int i = 0; i < t->children->size; ++i) {
        trie *child = array_at(t->children, i);
        trie_pprint(child, depth + 1);
    }
}


trie *trie_alloc(char c) {
    trie *t = (trie *)malloc(sizeof(trie));
    t->c = c;
    t->children = array_alloc(sizeof(trie), 32);
    return t;
}

void trie_free(trie *root) {
    if (!root) {
        return;
    }
    for (int i = 0; i < root->children->size; ++i) {
        trie_free(root);
    }
    array_free(root->children);
    free(root);
}

void trie_insert(trie *root, char *s) {
    if (!*s) {
        return;
    }
    for (int i = 0; i < root->children->size; ++i) {
        trie *child = (trie *)array_at(root->children, i);
        if (child->c == *s) {
            trie_insert(child, s+1);
            return;
        }
    }
    trie *child = trie_alloc(*s);
    trie_insert(child, s+1);
    int res = array_append(root->children, child);
    if (res == -1) {
        perror("Failed to append child!");
    }
}

int main() {
    trie *t = trie_alloc(TRIE_ROOT);
    trie_pprint(t, TRIE_ROOT);
    trie_insert(t, "tomato");
    trie_pprint(t, TRIE_ROOT);
    trie_insert(t, "total");
    trie_pprint(t, TRIE_ROOT);
}
