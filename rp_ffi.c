/*
 * rp_ffi.c — Flutter/dart:ffi wrapper implementation.
 *
 * Wraps the existing rp_padel_* API behind heap-allocated opaque handles
 * so Dart only deals with plain primitives (no struct layout mapping).
 */

#include "rp_ffi.h"
#include "padel_scoring.h"
#include <stdlib.h>
#include <string.h>

struct RPHandle {
    RPPadelState state;
};

RPHandle* rp_create(int mode, int golden_advantages, bool first_serve_a) {
    RPHandle* h = (RPHandle*)malloc(sizeof(RPHandle));
    if (!h) return NULL;
    RPPadelRules rules = {
        (RPPadelMode)mode,
        golden_advantages,
        first_serve_a
    };
    rp_padel_init(&h->state, rules);
    return h;
}

void rp_destroy(RPHandle* h) {
    if (h) free(h);
}

void rp_score_point(RPHandle* h, bool team_a) {
    if (!h) return;
    rp_padel_score_point(&h->state, team_a);
}

void rp_display(RPHandle* h, char* out_a, char* out_b) {
    if (!h) { out_a[0] = '\0'; out_b[0] = '\0'; return; }
    rp_padel_display_score(&h->state, out_a, out_b);
}

int rp_style(RPHandle* h) {
    if (!h) return 0;
    return (int)rp_padel_score_style(&h->state);
}

bool rp_serve_a(RPHandle* h)       { return h ? rp_padel_serve_a(&h->state)           : true;  }
int  rp_games_a(RPHandle* h)       { return h ? rp_padel_games_a(&h->state)            : 0;     }
int  rp_games_b(RPHandle* h)       { return h ? rp_padel_games_b(&h->state)            : 0;     }
int  rp_sets_a(RPHandle* h)        { return h ? rp_padel_sets_a(&h->state)             : 0;     }
int  rp_sets_b(RPHandle* h)        { return h ? rp_padel_sets_b(&h->state)             : 0;     }
bool rp_match_finished(RPHandle* h){ return h ? rp_padel_is_match_finished(&h->state)  : false; }
