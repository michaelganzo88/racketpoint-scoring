/*
 * padel_scoring.h — RacketPoint padel scoring engine (C99)
 *
 * Single source of truth for padel scoring logic across all platforms:
 *   - racketpoint-studio  (macOS SwiftUI, via SPM)
 *   - racketpoint-flutter (watchOS / iOS / Android, via SPM + dart:ffi)
 *   - racketpoint_esp32   (PlatformIO, included directly)
 *
 * Supported modes:
 *   RP_PADEL_MODE_CLASSIC              — unlimited advantages at deuce
 *   RP_PADEL_MODE_GOLDEN_POINT         — no advantage; next point at 40-40 wins
 *   RP_PADEL_MODE_ADVANTAGE_PLUS_GOLDEN — N advantages allowed, then golden point
 *
 * Display string format (rp_padel_display_score):
 *   Normal scoring  : "0" / "15" / "30" / "40"
 *   First 40-40     : "40" / "40"  (style = NORMAL)
 *   Advantage       : "AD1" / ""   (or "" / "AD1")
 *   Return to deuce : "D1" / "D1"  (style = DEUCE)
 *   Golden point    : "GP" / "GP"  (style = GOLDEN_POINT)
 *   Tie-break       : raw numbers, e.g. "4" / "3"
 *
 * See scoring.h for the umbrella include (adds future sports: pickleball, tennis).
 */

#pragma once
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ── Enums ────────────────────────────────────────────────────────────────── */

typedef enum {
    RP_PADEL_MODE_CLASSIC = 0,
    RP_PADEL_MODE_GOLDEN_POINT,
    RP_PADEL_MODE_ADVANTAGE_PLUS_GOLDEN
} RPPadelMode;

typedef enum {
    RP_PADEL_STYLE_NORMAL = 0,
    RP_PADEL_STYLE_DEUCE,        /* Dn: return to deuce after advantage — purple background */
    RP_PADEL_STYLE_GOLDEN_POINT  /* GP: golden point active — gold background */
} RPPadelScoreStyle;

/* ── Structs ──────────────────────────────────────────────────────────────── */

typedef struct {
    RPPadelMode mode;
    int         goldenAdvantages; /* 0-3; meaningful only with RP_PADEL_MODE_ADVANTAGE_PLUS_GOLDEN */
    bool        firstServeA;      /* true = Team A serves first */
} RPPadelRules;

/*
 * RPPadelState — full match state. Treat as opaque: read only via the
 * provided getter functions; write only via rp_padel_init / rp_padel_score_point.
 */
typedef struct {
    /* In-game points: 0=0pts, 1=15, 2=30, 3=40. */
    int  pointsA;
    int  pointsB;
    /* At most one of the two is true at any given time. */
    bool teamAHasAdvantage;
    bool teamBHasAdvantage;
    /* Reset to zero at the start of every new game. */
    int  advTaken;      /* advantages given since the first 40-40 in this game */
    int  deuceReturns;  /* times play returned to deuce after an advantage */
    /* Games in the current set. */
    int  gamesA;
    int  gamesB;
    /* Completed set counts. */
    int  setsWonA;
    int  setsWonB;
    /* Game scores per completed set (max 5 sets, padel is typically best of 3). */
    int  setHistoryA[5];
    int  setHistoryB[5];
    int  completedSets;
    /* True while a tie-break is in progress (triggered at 6-6 games). */
    bool tiebreak;
    /* True = Team A is currently serving. Alternates after every game. */
    bool serveA;
    /* Rules are stored inside the state for self-contained access. */
    RPPadelRules rules;
} RPPadelState;

/* ── Public API ───────────────────────────────────────────────────────────── */

/*
 * rp_padel_init — initialise the state.
 * Must be called before any other function.
 */
void rp_padel_init(RPPadelState* state, RPPadelRules rules);

/*
 * rp_padel_score_point — register a point scored by Team A (true) or Team B (false).
 *
 * Handles automatically:
 *   normal scoring → deuce → advantage → golden point
 *   game end → serve rotation → set end → tie-break
 */
void rp_padel_score_point(RPPadelState* state, bool teamA);

/*
 * rp_padel_display_score — fill outA and outB with display strings for the
 * current in-game score. Buffers must be at least 8 characters.
 *
 * Callers can translate these tags for localised display:
 *   "GP"  → "P. Oro" (Italian), "P. de Oro" (Spanish), …
 *   "D1"  → "Parità" / "Iguales" / "Deuce" …
 *   "AD1" → "Vantaggio A" / "Ventaja A" / "Advantage A" …
 */
void rp_padel_display_score(const RPPadelState* state, char* outA, char* outB);

/*
 * rp_padel_score_style — return the visual style for the score cells.
 * Use this to decide background colour on the scoreboard.
 */
RPPadelScoreStyle rp_padel_score_style(const RPPadelState* state);

/* Getters */
int  rp_padel_games_a(const RPPadelState* state);
int  rp_padel_games_b(const RPPadelState* state);
int  rp_padel_sets_a(const RPPadelState* state);
int  rp_padel_sets_b(const RPPadelState* state);
bool rp_padel_serve_a(const RPPadelState* state);

/*
 * rp_padel_is_match_finished — returns true when a team has won 2 sets
 * (standard padel best-of-3). Callers can override by inspecting
 * setsWonA / setsWonB directly for other formats.
 */
bool rp_padel_is_match_finished(const RPPadelState* state);

#ifdef __cplusplus
}
#endif
