/*
 * rp_ffi.h — Flutter/dart:ffi friendly wrapper for the padel scoring engine.
 *
 * Exposes a heap-allocated opaque handle so Dart never needs to map
 * RPPadelState's internal struct layout (arrays, bools, padding).
 * All parameters and return values are plain C primitives.
 *
 * Link: padel_scoring.c + rp_ffi.c
 */

#pragma once
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Opaque handle returned by rp_create. */
typedef struct RPHandle RPHandle;

/*
 * rp_create — allocate and initialise a new match state.
 *
 *   mode              0 = classic, 1 = golden_point, 2 = advantage_plus_golden
 *   golden_advantages 0-3; only used with mode == 2
 *   first_serve_a     true = Team A serves first
 *
 * Returns NULL on allocation failure (extremely unlikely).
 * Caller must release with rp_destroy().
 */
RPHandle* rp_create(int mode, int golden_advantages, bool first_serve_a);

/* rp_destroy — free a handle returned by rp_create. Safe to call with NULL. */
void rp_destroy(RPHandle* h);

/* rp_score_point — register a scored point. teamA = true → Team A scored. */
void rp_score_point(RPHandle* h, bool team_a);

/*
 * rp_display — fill outA[8] and outB[8] with the current in-game display strings.
 * Examples: "0","15","30","40","AD1","D1","GP"  (see padel_scoring.h for full spec).
 * Buffers must be at least 8 bytes.
 */
void rp_display(RPHandle* h, char* out_a, char* out_b);

/*
 * rp_style — visual style for the score cells:
 *   0 = normal  (white/grey background)
 *   1 = deuce   (purple background)
 *   2 = golden  (gold background)
 */
int rp_style(RPHandle* h);

/* State getters */
bool rp_serve_a(RPHandle* h);
int  rp_games_a(RPHandle* h);
int  rp_games_b(RPHandle* h);
int  rp_sets_a(RPHandle* h);
int  rp_sets_b(RPHandle* h);
bool rp_match_finished(RPHandle* h);

#ifdef __cplusplus
}
#endif
