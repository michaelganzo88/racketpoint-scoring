/*
 * scoring.h — RacketPoint multi-sport scoring engine (C99)
 *
 * Umbrella header: include this file to get all sport modules.
 * Each sport lives in its own header and implementation file,
 * with types and functions namespaced by sport:
 *
 *   padel_scoring.h / padel_scoring.c
 *     RPPadelState, RPPadelRules, RPPadelMode, RPPadelScoreStyle
 *     rp_padel_init, rp_padel_score_point, rp_padel_display_score, …
 *
 *   pickleball_scoring.h / pickleball_scoring.c  (future)
 *     RPPickleballState, RPPickleballRules, …
 *     rp_pickleball_init, rp_pickleball_score_point, …
 *
 *   tennis_scoring.h / tennis_scoring.c           (future)
 *     RPTennisState, RPTennisRules, …
 *     rp_tennis_init, rp_tennis_score_point, …
 *
 * Platforms that only need one sport can include the sport-specific
 * header directly instead of this umbrella.
 */

#pragma once

#include "padel_scoring.h"

/* Uncomment as each sport is implemented:
 * #include "pickleball_scoring.h"
 * #include "tennis_scoring.h"
 */
