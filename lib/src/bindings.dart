// ignore_for_file: non_constant_identifier_names, camel_case_types

import 'dart:ffi';
import 'dart:io';

// ── Load the native library ────────────────────────────────────────────────────

DynamicLibrary _openLib() {
  if (Platform.isMacOS) {
    return DynamicLibrary.open('libracketpoint_scoring.dylib');
  }
  if (Platform.isLinux || Platform.isAndroid) {
    return DynamicLibrary.open('libracketpoint_scoring.so');
  }
  if (Platform.isWindows) {
    return DynamicLibrary.open('racketpoint_scoring.dll');
  }
  if (Platform.isIOS) {
    return DynamicLibrary.process();
  }
  throw UnsupportedError(
      'racketpoint_scoring: unsupported platform ${Platform.operatingSystem}');
}

final DynamicLibrary _lib = _openLib();

// ── Native type aliases ────────────────────────────────────────────────────────

/// Opaque C handle (RPHandle*).
final class RPHandle extends Opaque {}

// ── Raw function lookups ───────────────────────────────────────────────────────

final RPHandle Function(int mode, int goldenAdvantages, bool firstServeA)
    rp_create = _lib.lookupFunction<
        Pointer<RPHandle> Function(Int32, Int32, Bool),
        RPHandle Function(int, int, bool)>('rp_create') as dynamic;

// We use Pointer<RPHandle> throughout so GC doesn't collect the native pointer.

final Pointer<RPHandle> Function(int mode, int goldenAdvantages,
    bool firstServeA) nativeCreate = _lib.lookupFunction<
    Pointer<RPHandle> Function(Int32, Int32, Bool),
    Pointer<RPHandle> Function(int, int, bool)>('rp_create');

final void Function(Pointer<RPHandle> h) nativeDestroy =
    _lib.lookupFunction<Void Function(Pointer<RPHandle>),
        void Function(Pointer<RPHandle>)>('rp_destroy');

final void Function(Pointer<RPHandle> h, bool teamA) nativeScorePoint =
    _lib.lookupFunction<Void Function(Pointer<RPHandle>, Bool),
        void Function(Pointer<RPHandle>, bool)>('rp_score_point');

final void Function(Pointer<RPHandle> h, Pointer<Char> outA,
    Pointer<Char> outB) nativeDisplay =
    _lib.lookupFunction<
        Void Function(Pointer<RPHandle>, Pointer<Char>, Pointer<Char>),
        void Function(Pointer<RPHandle>, Pointer<Char>,
            Pointer<Char>)>('rp_display');

final int Function(Pointer<RPHandle> h) nativeStyle =
    _lib.lookupFunction<Int32 Function(Pointer<RPHandle>),
        int Function(Pointer<RPHandle>)>('rp_style');

final bool Function(Pointer<RPHandle> h) nativeServeA =
    _lib.lookupFunction<Bool Function(Pointer<RPHandle>),
        bool Function(Pointer<RPHandle>)>('rp_serve_a');

final int Function(Pointer<RPHandle> h) nativeGamesA =
    _lib.lookupFunction<Int32 Function(Pointer<RPHandle>),
        int Function(Pointer<RPHandle>)>('rp_games_a');

final int Function(Pointer<RPHandle> h) nativeGamesB =
    _lib.lookupFunction<Int32 Function(Pointer<RPHandle>),
        int Function(Pointer<RPHandle>)>('rp_games_b');

final int Function(Pointer<RPHandle> h) nativeSetsA =
    _lib.lookupFunction<Int32 Function(Pointer<RPHandle>),
        int Function(Pointer<RPHandle>)>('rp_sets_a');

final int Function(Pointer<RPHandle> h) nativeSetsB =
    _lib.lookupFunction<Int32 Function(Pointer<RPHandle>),
        int Function(Pointer<RPHandle>)>('rp_sets_b');

final bool Function(Pointer<RPHandle> h) nativeMatchFinished =
    _lib.lookupFunction<Bool Function(Pointer<RPHandle>),
        bool Function(Pointer<RPHandle>)>('rp_match_finished');
