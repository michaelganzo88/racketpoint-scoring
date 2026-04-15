/*
 * test_padel_scoring.c — unit tests for the RacketPoint padel scoring engine
 *
 * Compile & run (from racketpoint-scoring/):
 *   cmake -B build && cmake --build build && ./build/test_padel_scoring
 *
 * Tests cover:
 *   1. Normal scoring:  0 → 15 → 30 → 40 → game
 *   2. Classic deuce:   40-40 → AD A → back to deuce → AD B → game B
 *   3. Golden point:    40-40 → GP → game (next point wins immediately)
 *   4. AdvPlus N=1:     40-40 → AD1 A → deuce → GP → game
 *   5. AdvPlus N=2:     40-40 → AD1 A → D1 → AD2 B → GP → game B
 *   6. Tie-break:       6-6 → TB → first to 7 with 2 lead
 *   7. Set scoring:     6 games → set win; 7-5 set
 *   8. Serve rotation:  alternates after each game; firstServeA respected
 *   9. Match finished
 *  10. Score style:     NORMAL / DEUCE / GOLDEN_POINT
 *  11. Full match accumulation
 */

#include "../padel_scoring.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* ── Minimal test framework ──────────────────────────────────────────────── */

static int tests_run    = 0;
static int tests_failed = 0;

#define ASSERT(cond, msg) do { \
    tests_run++; \
    if (!(cond)) { \
        tests_failed++; \
        printf("  FAIL [%s:%d] %s\n", __FILE__, __LINE__, (msg)); \
    } \
} while (0)

#define ASSERT_INT(a, b, msg) do { \
    tests_run++; \
    if ((a) != (b)) { \
        tests_failed++; \
        printf("  FAIL [%s:%d] %s — expected %d, got %d\n", \
               __FILE__, __LINE__, (msg), (b), (a)); \
    } \
} while (0)

#define ASSERT_STR(a, b, msg) do { \
    tests_run++; \
    if (strcmp((a), (b)) != 0) { \
        tests_failed++; \
        printf("  FAIL [%s:%d] %s — expected \"%s\", got \"%s\"\n", \
               __FILE__, __LINE__, (msg), (b), (a)); \
    } \
} while (0)

#define SECTION(name) printf("\n%s\n", (name))

/* ── Helpers ─────────────────────────────────────────────────────────────── */

static void score_n(RPPadelState* s, bool teamA, int n) {
    for (int i = 0; i < n; i++) rp_padel_score_point(s, teamA);
}

static void win_game_straight(RPPadelState* s, bool teamA) {
    score_n(s, teamA, 4);
}

static void win_set_straight(RPPadelState* s, bool teamA) {
    for (int g = 0; g < 6; g++) win_game_straight(s, teamA);
}

static void display(const RPPadelState* s, char* a, char* b) {
    rp_padel_display_score(s, a, b);
}

/* ── Tests ───────────────────────────────────────────────────────────────── */

static void test_normal_scoring(void) {
    SECTION("1. Normal scoring (0→15→30→40→game)");
    RPPadelState s;
    RPPadelRules r = { RP_PADEL_MODE_CLASSIC, 0, true };
    rp_padel_init(&s, r);

    char a[8], b[8];
    display(&s, a, b);
    ASSERT_STR(a, "0",  "score A starts at 0");
    ASSERT_STR(b, "0",  "score B starts at 0");

    rp_padel_score_point(&s, true);
    display(&s, a, b);
    ASSERT_STR(a, "15", "A at 15 after 1 point");
    ASSERT_STR(b, "0",  "B still at 0");

    rp_padel_score_point(&s, true);
    display(&s, a, b);
    ASSERT_STR(a, "30", "A at 30");

    rp_padel_score_point(&s, true);
    display(&s, a, b);
    ASSERT_STR(a, "40", "A at 40");

    rp_padel_score_point(&s, true);
    ASSERT_INT(rp_padel_games_a(&s), 1, "A wins first game");
    ASSERT_INT(rp_padel_games_b(&s), 0, "B still 0 games");

    display(&s, a, b);
    ASSERT_STR(a, "0", "in-game resets to 0 after game");
    ASSERT_STR(b, "0", "in-game resets to 0 after game (B)");
}

static void test_classic_deuce(void) {
    SECTION("2. Classic deuce: 40-40 → AD A → deuce → AD B → game B");
    RPPadelState s;
    RPPadelRules r = { RP_PADEL_MODE_CLASSIC, 0, true };
    rp_padel_init(&s, r);

    char a[8], b[8];

    score_n(&s, true,  3);
    score_n(&s, false, 3);
    display(&s, a, b);
    ASSERT_STR(a, "40", "first 40-40: A shows 40");
    ASSERT_STR(b, "40", "first 40-40: B shows 40");
    ASSERT(rp_padel_score_style(&s) == RP_PADEL_STYLE_NORMAL, "first 40-40: style NORMAL");

    rp_padel_score_point(&s, true);
    display(&s, a, b);
    ASSERT_STR(a, "AD1", "A has advantage AD1");
    ASSERT_STR(b, "",    "B shows empty during A advantage");

    rp_padel_score_point(&s, false);
    display(&s, a, b);
    ASSERT_STR(a, "D1", "deuce D1 for A");
    ASSERT_STR(b, "D1", "deuce D1 for B");
    ASSERT(rp_padel_score_style(&s) == RP_PADEL_STYLE_DEUCE, "deuce style after return");

    rp_padel_score_point(&s, false);
    display(&s, a, b);
    ASSERT_STR(a, "",    "A shows empty during B advantage");
    ASSERT_STR(b, "AD2", "B has advantage AD2");

    rp_padel_score_point(&s, false);
    ASSERT_INT(rp_padel_games_b(&s), 1, "B wins the game");
    ASSERT_INT(rp_padel_games_a(&s), 0, "A still 0 games");
}

static void test_golden_point(void) {
    SECTION("3. Golden point (noAdvantage): 40-40 → GP → game immediately");
    RPPadelState s;
    RPPadelRules r = { RP_PADEL_MODE_GOLDEN_POINT, 0, true };
    rp_padel_init(&s, r);

    char a[8], b[8];

    score_n(&s, true,  3);
    score_n(&s, false, 3);
    display(&s, a, b);
    ASSERT_STR(a, "GP", "golden point: A shows GP");
    ASSERT_STR(b, "GP", "golden point: B shows GP");
    ASSERT(rp_padel_score_style(&s) == RP_PADEL_STYLE_GOLDEN_POINT, "style is GOLDEN_POINT");

    rp_padel_score_point(&s, true);
    ASSERT_INT(rp_padel_games_a(&s), 1, "A wins game via golden point");
}

static void test_adv_plus_golden_n1(void) {
    SECTION("4. advantagePlusGolden N=1: 40-40 → AD1 → deuce → GP → game");
    RPPadelState s;
    RPPadelRules r = { RP_PADEL_MODE_ADVANTAGE_PLUS_GOLDEN, 1, true };
    rp_padel_init(&s, r);

    char a[8], b[8];

    score_n(&s, true,  3);
    score_n(&s, false, 3);
    display(&s, a, b);
    ASSERT_STR(a, "40", "N=1: first 40-40 shows normal 40");
    ASSERT(rp_padel_score_style(&s) == RP_PADEL_STYLE_NORMAL, "N=1: first 40-40 style NORMAL");

    rp_padel_score_point(&s, true);
    display(&s, a, b);
    ASSERT_STR(a, "AD1", "N=1: A shows AD1");
    ASSERT_STR(b, "",    "N=1: B shows empty");

    rp_padel_score_point(&s, false);
    display(&s, a, b);
    ASSERT_STR(a, "GP", "N=1: after return to deuce, GP active");
    ASSERT_STR(b, "GP", "N=1: B also shows GP");
    ASSERT(rp_padel_score_style(&s) == RP_PADEL_STYLE_GOLDEN_POINT, "N=1: style GOLDEN_POINT");

    rp_padel_score_point(&s, false);
    ASSERT_INT(rp_padel_games_b(&s), 1, "N=1: B wins game via golden point");
}

static void test_adv_plus_golden_n2(void) {
    SECTION("5. advantagePlusGolden N=2: 40-40 → AD1 A → D1 → AD2 B → GP → game B");
    RPPadelState s;
    RPPadelRules r = { RP_PADEL_MODE_ADVANTAGE_PLUS_GOLDEN, 2, true };
    rp_padel_init(&s, r);

    char a[8], b[8];

    score_n(&s, true, 3); score_n(&s, false, 3);

    rp_padel_score_point(&s, true);
    display(&s, a, b);
    ASSERT_STR(a, "AD1", "N=2: AD1 for A");

    rp_padel_score_point(&s, false);
    display(&s, a, b);
    ASSERT_STR(a, "D1", "N=2: D1 after first return");
    ASSERT(rp_padel_score_style(&s) == RP_PADEL_STYLE_DEUCE, "N=2: style DEUCE at D1");
    ASSERT(rp_padel_score_style(&s) != RP_PADEL_STYLE_GOLDEN_POINT, "N=2: NOT golden yet at D1");

    rp_padel_score_point(&s, false);
    display(&s, a, b);
    ASSERT_STR(b, "AD2", "N=2: AD2 for B");
    ASSERT_STR(a, "",    "N=2: A shows empty");

    rp_padel_score_point(&s, true);
    display(&s, a, b);
    ASSERT_STR(a, "GP", "N=2: GP after second return");
    ASSERT(rp_padel_score_style(&s) == RP_PADEL_STYLE_GOLDEN_POINT, "N=2: style GOLDEN_POINT");

    rp_padel_score_point(&s, false);
    ASSERT_INT(rp_padel_games_b(&s), 1, "N=2: B wins game via golden point");
}

static void test_tiebreak(void) {
    SECTION("6. Tie-break: 6-6 → TB → first to 7 with 2 lead");
    RPPadelState s;
    RPPadelRules r = { RP_PADEL_MODE_CLASSIC, 0, true };
    rp_padel_init(&s, r);

    for (int i = 0; i < 6; i++) {
        win_game_straight(&s, true);
        win_game_straight(&s, false);
    }
    ASSERT_INT(rp_padel_games_a(&s), 6, "6-6: A has 6 games");
    ASSERT_INT(rp_padel_games_b(&s), 6, "6-6: B has 6 games");
    ASSERT(s.tiebreak, "tie-break flag set at 6-6");

    char a[8], b[8];

    score_n(&s, true, 6);
    score_n(&s, false, 6);
    display(&s, a, b);
    ASSERT_STR(a, "6", "TB: A shows raw 6");
    ASSERT_STR(b, "6", "TB: B shows raw 6");
    ASSERT(s.tiebreak, "still in tie-break at 6-6 TB");

    rp_padel_score_point(&s, true);
    ASSERT(s.tiebreak, "still in tie-break at 7-6 TB");

    rp_padel_score_point(&s, true);
    ASSERT(!s.tiebreak, "tie-break ends at 8-6");
    ASSERT_INT(rp_padel_sets_a(&s), 1, "A wins set via tie-break");
    ASSERT_INT(rp_padel_games_a(&s), 0, "games reset after set");
    ASSERT_INT(rp_padel_games_b(&s), 0, "games reset after set (B)");
}

static void test_set_scoring(void) {
    SECTION("7. Set scoring: 6-0 set win; 7-5 set win");
    RPPadelState s;
    RPPadelRules r = { RP_PADEL_MODE_CLASSIC, 0, true };
    rp_padel_init(&s, r);

    win_set_straight(&s, true);
    ASSERT_INT(rp_padel_sets_a(&s), 1, "A wins first set 6-0");
    ASSERT_INT(rp_padel_games_a(&s), 0, "games reset after set");

    for (int i = 0; i < 5; i++) win_game_straight(&s, true);
    for (int i = 0; i < 5; i++) win_game_straight(&s, false);
    win_game_straight(&s, true);
    ASSERT_INT(rp_padel_games_a(&s), 6, "6-5: A at 6 games");
    ASSERT_INT(rp_padel_games_b(&s), 5, "6-5: B at 5 games");
    ASSERT_INT(rp_padel_sets_a(&s), 1, "no new set yet at 6-5");

    win_game_straight(&s, false);
    ASSERT(s.tiebreak, "tiebreak at 6-6");

    score_n(&s, false, 7);
    ASSERT_INT(rp_padel_sets_b(&s), 1, "B wins set via tiebreak");
    ASSERT_INT(rp_padel_sets_a(&s), 1, "A still 1 set");

    for (int i = 0; i < 5; i++) win_game_straight(&s, true);
    for (int i = 0; i < 5; i++) win_game_straight(&s, false);
    win_game_straight(&s, true);  /* 6-5 */
    win_game_straight(&s, true);  /* 7-5 → set */
    ASSERT_INT(rp_padel_sets_a(&s), 2, "A wins set 7-5");
    ASSERT_INT(rp_padel_games_a(&s), 0, "games reset after 7-5 set");
}

static void test_serve_rotation(void) {
    SECTION("8. Serve rotation: alternates after each game; firstServeA respected");
    RPPadelState s;

    RPPadelRules r = { RP_PADEL_MODE_CLASSIC, 0, true };
    rp_padel_init(&s, r);
    ASSERT(rp_padel_serve_a(&s), "firstServeA=true → A serves first");

    win_game_straight(&s, true);
    ASSERT(!rp_padel_serve_a(&s), "after game 1: B serves");

    win_game_straight(&s, true);
    ASSERT(rp_padel_serve_a(&s), "after game 2: A serves again");

    RPPadelRules r2 = { RP_PADEL_MODE_CLASSIC, 0, false };
    rp_padel_init(&s, r2);
    ASSERT(!rp_padel_serve_a(&s), "firstServeA=false → B serves first");

    win_game_straight(&s, false);
    ASSERT(rp_padel_serve_a(&s), "after game 1 (B first): A serves");
}

static void test_match_finished(void) {
    SECTION("9. Match finished (best-of-3)");
    RPPadelState s;
    RPPadelRules r = { RP_PADEL_MODE_CLASSIC, 0, true };
    rp_padel_init(&s, r);

    ASSERT(!rp_padel_is_match_finished(&s), "not finished at start");

    win_set_straight(&s, true);
    ASSERT(!rp_padel_is_match_finished(&s), "not finished after 1 set");

    win_set_straight(&s, true);
    ASSERT(rp_padel_is_match_finished(&s), "finished after 2 sets");
}

static void test_score_style_transitions(void) {
    SECTION("10. Score style transitions");
    RPPadelState s;

    RPPadelRules rc = { RP_PADEL_MODE_CLASSIC, 0, true };
    rp_padel_init(&s, rc);
    score_n(&s, true, 3); score_n(&s, false, 3);
    ASSERT(rp_padel_score_style(&s) == RP_PADEL_STYLE_NORMAL, "first 40-40: NORMAL");

    rp_padel_score_point(&s, true);
    ASSERT(rp_padel_score_style(&s) == RP_PADEL_STYLE_NORMAL, "during AD: NORMAL");

    rp_padel_score_point(&s, false);
    ASSERT(rp_padel_score_style(&s) == RP_PADEL_STYLE_DEUCE, "D1: DEUCE");

    RPPadelRules rg = { RP_PADEL_MODE_GOLDEN_POINT, 0, true };
    rp_padel_init(&s, rg);
    score_n(&s, true, 3); score_n(&s, false, 3);
    ASSERT(rp_padel_score_style(&s) == RP_PADEL_STYLE_GOLDEN_POINT, "40-40 GP mode: GOLDEN_POINT");

    RPPadelRules ra = { RP_PADEL_MODE_ADVANTAGE_PLUS_GOLDEN, 1, true };
    rp_padel_init(&s, ra);
    score_n(&s, true, 3); score_n(&s, false, 3);
    ASSERT(rp_padel_score_style(&s) == RP_PADEL_STYLE_NORMAL, "APG N=1 first 40-40: NORMAL");
    rp_padel_score_point(&s, true);
    ASSERT(rp_padel_score_style(&s) == RP_PADEL_STYLE_NORMAL, "APG N=1 during AD: NORMAL");
    rp_padel_score_point(&s, false);
    ASSERT(rp_padel_score_style(&s) == RP_PADEL_STYLE_GOLDEN_POINT, "APG N=1 after return: GOLDEN_POINT");
}

static void test_full_match_score_accumulation(void) {
    SECTION("11. Full match: A wins 6-0, 6-0");
    RPPadelState s;
    RPPadelRules r = { RP_PADEL_MODE_CLASSIC, 0, true };
    rp_padel_init(&s, r);

    win_set_straight(&s, true);
    ASSERT_INT(rp_padel_sets_a(&s), 1, "A wins first set");
    ASSERT_INT(s.setHistoryA[0], 6, "set history A[0] = 6");
    ASSERT_INT(s.setHistoryB[0], 0, "set history B[0] = 0");

    win_set_straight(&s, true);
    ASSERT_INT(rp_padel_sets_a(&s), 2, "A wins second set");
    ASSERT(rp_padel_is_match_finished(&s), "match finished after 2-0 sets");
    ASSERT_INT(s.completedSets, 2, "2 completed sets");
}

/* ── Main ─────────────────────────────────────────────────────────────────── */

int main(void) {
    printf("RacketPoint Padel Scoring Engine — Test Suite\n");
    printf("===============================================\n");

    test_normal_scoring();
    test_classic_deuce();
    test_golden_point();
    test_adv_plus_golden_n1();
    test_adv_plus_golden_n2();
    test_tiebreak();
    test_set_scoring();
    test_serve_rotation();
    test_match_finished();
    test_score_style_transitions();
    test_full_match_score_accumulation();

    printf("\n===============================================\n");
    printf("Results: %d/%d tests passed", tests_run - tests_failed, tests_run);
    if (tests_failed == 0) {
        printf(" ✓\n");
    } else {
        printf(" (%d failed)\n", tests_failed);
    }

    return tests_failed == 0 ? 0 : 1;
}
