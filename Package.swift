// swift-tools-version: 5.9
import PackageDescription

let package = Package(
    name: "RacketPointScoring",
    platforms: [
        .iOS(.v13),
        .watchOS(.v8)
    ],
    products: [
        // dynamic so that dlsym(RTLD_DEFAULT) can find rp_* symbols at runtime
        .library(
            name: "RacketPointScoring",
            type: .dynamic,
            targets: ["RacketPointScoring"]
        )
    ],
    targets: [
        .target(
            name: "RacketPointScoring",
            path: ".",
            exclude: ["tests", "CMakeLists.txt", "pubspec.yaml", "lib", "linux", "macos", "windows"],
            sources: ["padel_scoring.c", "rp_ffi.c"],
            publicHeadersPath: "include"
        )
    ]
)
