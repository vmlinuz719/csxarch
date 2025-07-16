#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#include "asm.h"

// asmlabel - Label registration and resolution

#define START_LABELS 64
#define ADD_LABELS 32

struct label_list *init_label_list() {
    struct label_list *ll = malloc(sizeof(struct label_list));
    ll->n_labels = 0;
    ll->current_max_labels = START_LABELS;
    ll->labels = malloc(sizeof(struct label_def) * START_LABELS);
    return ll;
}

void destroy_label_list(struct label_list *ll) {
    free(ll->labels);
    free(ll);
}

uint64_t *search_label(struct label_list *ll, char *label) {
    for (int i = 0; i < ll->n_labels; i++) {
        if (!strcmp(ll->labels[i].label, label)) {
            return &(ll->labels[i].value);
        }
    }
    return NULL;
}

struct label_def *register_label(
    struct label_list *ll,
    char *label,
    uint64_t value
) {
    if (search_label(ll, label) != NULL) return NULL;
    if (ll->n_labels == ll->current_max_labels) {
        ll->labels = realloc(
            ll->labels,
            (ll->current_max_labels + ADD_LABELS) * sizeof(struct label_def)
        );
        ll->current_max_labels += ADD_LABELS;
    }
    strcpy(ll->labels[ll->n_labels].label, label);
    ll->labels[ll->n_labels].value = value;
    return &(ll->labels[ll->n_labels++]);
}