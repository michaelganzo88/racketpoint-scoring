/*
 * padel_scoring.c — RacketPoint padel scoring engine (C99)
 *
 * Logic derived from PadelMatchState (racketpoint-studio/VideoOverlayService.swift),
 * which is the most complete and battle-tested implementation of the three.
 *
 * Do NOT add platform-specific code here (no millis(), no Serial, no NSLog, …).
 * This file must compile cleanly on any C99-compatible toolchain.
 */

#include "padel_scoring.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>  /* abs() */

/* ── Private helpers ──────────────────────────────────────────────────────── */

static bool is_gp_active(const RPPadelState* s) {
    if (s->rules.mode == RP_PADEL_MODE_GOLDEN_POINT) return true;
    if (s->rules.mode == RP_PADEL_MODE_ADVANTAGE_PLUS_GOLDEN &&
        s->advTaken >= s->rules.goldenAdvantages) return true;
    return false;
}

/*
 * win_set — record a completed set and reset game counters.
 * The tiebreak path writes its own set history before calling this,
 * so here we only update history for normal sets.
 */
static void win_set(RPPadelState* s, bool teamA) {
    if (s->completedSets < 5) {
        s->setHistoryA[s->completedSets] = s->gamesA;
        s->setHistoryB[s->completedSets] = s->gamesB;
        s->completedSets++;
    }
    if (teamA) s->setsWonA++; else s->setsWonB++;
    s->gamesA = 0;
    s->gamesB = 0;
}

/*
 * win_game — increment the game counter for the winning team, reset
 * all in-game state, flip serve, then check for set / tie-break.
 */
static void win_game(RPPadelState* s, bool teamA) {
    if (teamA) s->gamesA++; else s->gamesB++;

    /* Reset in-game state */
    s->pointsA           = 0;
    s->pointsB           = 0;
    s->teamAHasAdvantage = false;
    s->teamBHasAdvantage = false;
    s->advTaken          = 0;
    s->deuceReturns      = 0;
    s->tiebreak          = false;

    /* Serve alternates after every game */
    s->serveA = !s->serveA;

    /* Check set end:
     *   - first to 6 with ≥2 game lead  (e.g. 6-0 through 6-4)
     *   - 7-5
     *   - 7-6 is handled by the tie-break path, not here
     */
    int a = s->gamesA, b = s->gamesB;
    int winner = teamA ? a : b;
    int loser  = teamA ? b : a;

    if ((winner >= 6 && winner - loser >= 2) || winner == 7) {
        win_set(s, teamA);
    } else if (a == 6 && b == 6) {
        s->tiebreak = true;
    }
}

/* ── Public functions ─────────────────────────────────────────────────────── */

void rp_padel_init(RPPadelState* state, RPPadelRules rules) {
    memset(state, 0, sizeof(RPPadelState));
    state->rules  = rules;
    state->serveA = rules.firstServeA;
}

void rp_padel_score_point(RPPadelState* state, bool teamA) {
    RPPadelState* s = state;

    /* ── Tie-break ────────────────────────────────────────────────────────── */
    if (s->tiebreak) {
        if (teamA) s->pointsA++; else s->pointsB++;

        int a = s->pointsA, b = s->pointsB;
        /* Tie-break ends at ≥7 with a 2-point lead */
        if ((a >= 7 || b >= 7) && abs(a - b) >= 2) {
            /* Record set as 7-6 / 6-7 */
            if (s->completedSets < 5) {
                s->setHistoryA[s->completedSets] = teamA ? 7 : 6;
                s->setHistoryB[s->completedSets] = teamA ? 6 : 7;
                s->completedSets++;
            }
            if (teamA) s->setsWonA++; else s->setsWonB++;

            /* Reset for next set */
            s->pointsA           = 0;
            s->pointsB           = 0;
            s->gamesA            = 0;
            s->gamesB            = 0;
            s->tiebreak          = false;
            s->advTaken          = 0;
            s->deuceReturns      = 0;
            s->teamAHasAdvantage = false;
            s->teamBHasAdvantage = false;
            /* Serve alternates at set start */
            s->serveA = !s->serveA;
        }
        return;
    }

    /* ── Active advantage ─────────────────────────────────────────────────── */
    if (s->teamAHasAdvantage || s->teamBHasAdvantage) {
        if (teamA) {
            if (s->teamAHasAdvantage) {
                win_game(s, true);          /* A confirms advantage → game A */
            } else {
                s->teamBHasAdvantage = false; /* B loses advantage → back to deuce */
                s->deuceReturns++;
            }
        } else {
            if (s->teamBHasAdvantage) {
                win_game(s, false);         /* B confirms advantage → game B */
            } else {
                s->teamAHasAdvantage = false; /* A loses advantage → back to deuce */
                s->deuceReturns++;
            }
        }
        return;
    }

    /* ── Deuce (3-3) ──────────────────────────────────────────────────────── */
    if (s->pointsA == 3 && s->pointsB == 3) {
        if (is_gp_active(s)) {
            win_game(s, teamA);             /* Golden point: scoring team wins immediately */
        } else {
            s->advTaken++;
            if (teamA) s->teamAHasAdvantage = true;
            else       s->teamBHasAdvantage = true;
        }
        return;
    }

    /* ── Normal scoring ───────────────────────────────────────────────────── */
    if (teamA) {
        if (s->pointsA >= 3 && s->pointsB < 3) win_game(s, true);
        else s->pointsA++;
    } else {
        if (s->pointsB >= 3 && s->pointsA < 3) win_game(s, false);
        else s->pointsB++;
    }
}

void rp_padel_display_score(const RPPadelState* state, char* outA, char* outB) {
    const RPPadelState* s = state;

    /* Tie-break: show raw point totals */
    if (s->tiebreak) {
        snprintf(outA, 8, "%d", s->pointsA);
        snprintf(outB, 8, "%d", s->pointsB);
        return;
    }

    /* Advantage: one side shows "ADn", the other is empty */
    if (s->teamAHasAdvantage) {
        snprintf(outA, 8, "AD%d", s->advTaken);
        outB[0] = '\0';
        return;
    }
    if (s->teamBHasAdvantage) {
        outA[0] = '\0';
        snprintf(outB, 8, "AD%d", s->advTaken);
        return;
    }

    /* Deuce (3-3) */
    if (s->pointsA == 3 && s->pointsB == 3) {
        if (is_gp_active(s)) {
            snprintf(outA, 8, "GP");
            snprintf(outB, 8, "GP");
        } else if (s->advTaken == 0) {
            /* First 40-40: display as normal "40" */
            snprintf(outA, 8, "40");
            snprintf(outB, 8, "40");
        } else {
            /* Return to deuce after ≥1 advantage: show "D{n}" */
            snprintf(outA, 8, "D%d", s->deuceReturns);
            snprintf(outB, 8, "D%d", s->deuceReturns);
        }
        return;
    }

    /* Normal scoring */
    static const char* const labels[] = { "0", "15", "30", "40" };
    int a = s->pointsA < 4 ? s->pointsA : 3;
    int b = s->pointsB < 4 ? s->pointsB : 3;
    snprintf(outA, 8, "%s", labels[a]);
    snprintf(outB, 8, "%s", labels[b]);
}

RPPadelScoreStyle rp_padel_score_style(const RPPadelState* state) {
    const RPPadelState* s = state;
    /* During advantage: cells are normal (one ADn, one empty) */
    if (s->teamAHasAdvantage || s->teamBHasAdvantage) return RP_PADEL_STYLE_NORMAL;
    if (s->pointsA == 3 && s->pointsB == 3) {
        if (is_gp_active(s))   return RP_PADEL_STYLE_GOLDEN_POINT;
        if (s->advTaken > 0)   return RP_PADEL_STYLE_DEUCE;
        /* First 40-40: normal style */
    }
    return RP_PADEL_STYLE_NORMAL;
}

int  rp_padel_games_a(const RPPadelState* state)           { return state->gamesA;    }
int  rp_padel_games_b(const RPPadelState* state)           { return state->gamesB;    }
int  rp_padel_sets_a(const RPPadelState* state)            { return state->setsWonA;  }
int  rp_padel_sets_b(const RPPadelState* state)            { return state->setsWonB;  }
bool rp_padel_serve_a(const RPPadelState* state)           { return state->serveA;    }

bool rp_padel_is_match_finished(const RPPadelState* state) {
    /* Standard padel best-of-3: first to win 2 sets */
    return state->setsWonA >= 2 || state->setsWonB >= 2;
}
