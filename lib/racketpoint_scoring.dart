/// RacketPoint padel scoring engine — Dart FFI bindings.
///
/// Usage:
/// ```dart
/// final match = PadelScoring(
///   mode: RPPadelMode.classic,
///   firstServeA: true,
/// );
/// match.scorePoint(true);   // Team A scores
/// final s = match.displayScore(); // (scoreA: '15', scoreB: '0')
/// match.dispose();
/// ```
library racketpoint_scoring;

import 'dart:ffi';
import 'package:ffi/ffi.dart';
import 'src/bindings.dart';

export 'src/bindings.dart' show RPHandle;

// ── Enums ──────────────────────────────────────────────────────────────────────

enum RPPadelMode {
  classic,              // unlimited advantages
  goldenPoint,          // golden point at deuce
  advantagePlusGolden,  // N advantages, then golden point
}

enum RPPadelScoreStyle {
  normal,      // standard background
  deuce,       // purple background (Dn)
  goldenPoint, // gold background (GP)
}

// ── Public API ─────────────────────────────────────────────────────────────────

/// Mutable padel match state. Create one instance per match/simulation.
/// Always call [dispose] when done to free native memory.
class PadelScoring {
  final Pointer<RPHandle> _h;
  bool _disposed = false;

  PadelScoring._raw(this._h);

  factory PadelScoring({
    RPPadelMode mode = RPPadelMode.classic,
    int goldenAdvantages = 0,
    bool firstServeA = true,
  }) {
    final h = nativeCreate(mode.index, goldenAdvantages, firstServeA);
    if (h.address == 0) throw StateError('rp_create returned NULL');
    return PadelScoring._raw(h);
  }

  void dispose() {
    if (_disposed) return;
    nativeDestroy(_h);
    _disposed = true;
  }

  // ── Mutations ──────────────────────────────────────────────────────────────

  void scorePoint(bool teamA) {
    _check();
    nativeScorePoint(_h, teamA);
  }

  // ── Getters ────────────────────────────────────────────────────────────────

  /// Returns `(scoreA, scoreB)` as display strings,
  /// e.g. `('15', '0')`, `('AD1', '')`, `('GP', 'GP')`, `('D1', 'D1')`.
  ({String scoreA, String scoreB}) displayScore() {
    _check();
    final pA = calloc<Char>(8);
    final pB = calloc<Char>(8);
    try {
      nativeDisplay(_h, pA, pB);
      return (
        scoreA: pA.cast<Utf8>().toDartString(),
        scoreB: pB.cast<Utf8>().toDartString(),
      );
    } finally {
      calloc.free(pA);
      calloc.free(pB);
    }
  }

  RPPadelScoreStyle get scoreStyle {
    _check();
    return RPPadelScoreStyle.values[nativeStyle(_h)];
  }

  bool get serveA        { _check(); return nativeServeA(_h); }
  int  get gamesA        { _check(); return nativeGamesA(_h); }
  int  get gamesB        { _check(); return nativeGamesB(_h); }
  int  get setsA         { _check(); return nativeSetsA(_h);  }
  int  get setsB         { _check(); return nativeSetsB(_h);  }
  bool get matchFinished { _check(); return nativeMatchFinished(_h); }

  void _check() {
    if (_disposed) throw StateError('PadelScoring has been disposed');
  }
}
